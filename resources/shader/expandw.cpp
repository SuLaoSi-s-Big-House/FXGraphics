#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
    #define FX_WINDOWS
#else
    #error "Unsupported platform!\n"
#endif

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <cassert>
#include <sstream>
#include <fstream>
#include <mutex>
#include <future>
#include <chrono>

#ifdef FX_WINDOWS
#include <Windows.h>
#undef min
#undef max
#endif

std::vector<std::string> headerFiles;
std::unordered_map<std::string, std::string> headerFileContents;
std::vector<std::string> shaderFiles;

std::mutex readMutex;
std::mutex writeMutex;
unsigned int index = 0;

constexpr unsigned char THREAD_NUM = 4;
constexpr unsigned char FILE_NUM_ONCE = 10;

inline void divideFileName(const std::string& fullName, std::string& name, std::string& ext)
{
    assert(fullName.length() >= 3);

    auto pos = fullName.find_last_of('.');
    if (pos == std::string::npos)
    {
        return;
    }

    name = fullName.substr(0, pos);
    ext = fullName.substr(pos + 1);
}

inline bool isHeaderFile(const std::string& ext)
{
    return ext == "h";
}

inline bool isShaderFile(const std::string& ext)
{
    return ext == "vert" || ext == "frag" || ext == "geom" || ext == "comp" || ext == "tese" || ext == "tesc";
}

#ifdef FX_WINDOWS
inline void collectAllFiles()
{
    WIN32_FIND_DATAA fileData;
    HANDLE handle = FindFirstFileA("./*.*", &fileData);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // 不是文件（是文件夹），则跳过
            {
                continue;
            }

            std::string fullName(fileData.cFileName);
            if (fullName.length() < 2)
            {
                continue;
            }

            std::string name;
            std::string ext;

            divideFileName(fileData.cFileName, name, ext);

            if (name.empty() || (ext.length() != 1 && ext.length() != 4))
            {
                continue;
            }

            if (isHeaderFile(ext))
            {
                headerFiles.emplace_back(std::move(fullName));
            }
            else if (isShaderFile(ext))
            {
                shaderFiles.emplace_back(std::move(fullName));
            }

        } while (FindNextFileA(handle, &fileData));
        FindClose(handle);
    }
}
#endif

inline std::string readOneFile(const std::string& name)
{
    assert(name.empty() == false);

    std::ifstream ifs;
    ifs.open(name);
    if (ifs.is_open() == false)
    {
        assert(ifs.is_open() == false);
        return std::string();
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    auto source = buffer.str();
    ifs.close();

    if (source.empty())
    {
        return source;
    }

    int i = 0;
    for (; i < source.size(); i++)
    {
        if (std::isprint(static_cast<unsigned char>(source[i])))
        {
            break;
        }
    }

    return source.substr(i);
}

inline void readHeaderFilesThread()
{
    std::array<std::string, FILE_NUM_ONCE> filesToRead;
    std::array<std::string, FILE_NUM_ONCE> contents;

    while (true)
    {
        unsigned char num = 0;
        {
            std::lock_guard<std::mutex> guard(readMutex);
            if (index >= headerFiles.size())
            {
                return;
            }

            num = static_cast<unsigned char>(std::min(static_cast<unsigned long long>(FILE_NUM_ONCE), headerFiles.size() - index));
            for (decltype(num) i = 0; i < num; i++)
            {
                filesToRead[i] = headerFiles[index + i];
            }
            index += num;
        }

        for (decltype(num) i = 0; i < num; i++)
        {
            contents[i] = std::move(readOneFile(filesToRead[i]));
        }

        std::lock_guard<std::mutex> guard(writeMutex);
        for (decltype(num) i = 0; i < num; i++)
        {
            headerFileContents.insert({ filesToRead[i], contents[i] });
        }
    }
}

inline void readAllHeaderFiles()
{
    assert(headerFiles.empty() == false);

    if (headerFiles.size() < 20)
    {
        for (auto& name : headerFiles)
        {
            auto source = readOneFile(name);
            headerFileContents.insert({ name, std::move(source) });
        }
    }
    else
    {
        index = 0;

        std::array<std::future<void>, THREAD_NUM> futures;
        for (int i = 0; i < THREAD_NUM; i++)
        {
            futures[i] = std::async(std::launch::async, readHeaderFilesThread);
        }

        for (int i = 0; i < THREAD_NUM; i++)
        {
            futures[i].get();
        }
    }
}

inline void replaceLine(std::string& line)
{
    if (line.size() < 13)
    {
        return;
    }

    unsigned int i = 0;
    unsigned int j = static_cast<unsigned int>(line.size() - 1);
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
    {
        i++;
    }
    while (j >= 0 && (line[j] == ' ' || line[j] == '\t'))
    {
        j--;
    }

    if (i >= line.size() || j < 0)
    {
        return;
    }
    if (j - i < 13)
    {
        return;
    }

    constexpr char frontStr[] = "#include \"";
    constexpr char backStr[] = ".h\"";

    if (line.compare(i, strlen(frontStr), frontStr) != 0 ||
        line.compare(j - strlen(backStr) + 1, strlen(backStr), backStr) != 0)
    {
        return;
    }

    auto name = line.substr(i + strlen(frontStr), j - i - strlen(frontStr));
    if (name.empty())
    {
        return;
    }

    auto itr = headerFileContents.find(name);
    if (itr == headerFileContents.end())
    {
        return;
    }

    line = itr->second;
}

inline void replaceOneShaderContent(const std::string& name)
{
    auto source = readOneFile(name);
    if (source.empty())
    {
        return;
    }

    std::istringstream iss(source);
    std::string result;
    result.reserve(source.size());

    std::string line;
    while (std::getline(iss, line))
    {
        replaceLine(line);
        result += line;
        result += '\n';
    }

    std::ofstream ofs(name);
    if (ofs.is_open())
    {
        ofs << result;
        ofs.close();
    }
}

inline void replaceShaderContentThread()
{
    std::array<std::string, FILE_NUM_ONCE> filesToReplace;

    while (true)
    {
        unsigned char num = 0;
        {
            std::lock_guard<std::mutex> guard(readMutex);
            if (index >= shaderFiles.size())
            {
                return;
            }

            num = static_cast<unsigned char>(std::min(static_cast<unsigned long long>(FILE_NUM_ONCE), shaderFiles.size() - index));
            for (decltype(num) i = 0; i < num; i++)
            {
                filesToReplace[i] = shaderFiles[index + i];
            }
            index += num;
        }

        for (decltype(num) i = 0; i < num; i++)
        {
            replaceOneShaderContent(filesToReplace[i]);
        }
    }
}

inline void replaceAllShaderContent()
{
    assert(shaderFiles.empty() == false);

    if (shaderFiles.size() < 20)
    {
        for (auto& name : shaderFiles)
        {
            replaceOneShaderContent(name);
        }
    }
    else
    {
        index = 0;

        std::array<std::future<void>, THREAD_NUM> futures;
        for (int i = 0; i < THREAD_NUM; i++)
        {
            futures[i] = std::async(std::launch::async, replaceShaderContentThread);
        }

        for (int i = 0; i < THREAD_NUM; i++)
        {
            futures[i].get();
        }
    }
}

int main()
{
    auto start = std::chrono::steady_clock::now();

    collectAllFiles();

    if (headerFiles.empty() || shaderFiles.empty())
    {
        std::cout << "Nothing to replace." << std::endl;
        return 0;
    }

    readAllHeaderFiles();

    replaceAllShaderContent();

    auto end = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Insert " << headerFiles.size() << " headers into " << shaderFiles.size() << " shaders, used " << dur.count() << " ms." << std::endl;

    return 0;
}

