#include "basic_image.h"

#include <cstring>
#include <assert.h>
#include "basic_log.h"

namespace FX {

    BasicImage::BasicImage(unsigned int width, unsigned int height, unsigned char channels, const unsigned char* pData)
    {
        if (width > 0 && height > 0 && channels > 0)
        {
            m_pData.reset(new unsigned char[width * height * channels]);
            m_width = width;
            m_height = height;
            m_channels = channels;

            if (pData != nullptr)
            {
                memcpy(m_pData.get(), pData, sizeof(unsigned char) * width * height * channels);
            }
            else
            {
                memset(m_pData.get(), 0, sizeof(unsigned char) * width * height * channels);
            }
        }
        else
        {
            BasicLog::out(BasicLog::kWarn, "Invalid data when creating image.");
        }
    }

    void BasicImage::setData(unsigned int width, unsigned int height, unsigned char channels, const unsigned char* pData)
    {
        if (width > 0 && height > 0 && channels > 0)
        {
            m_pData.reset(new unsigned char[width * height * channels]);
            m_width = width;
            m_height = height;
            m_channels = channels;

            if (pData != nullptr)
            {
                memcpy(m_pData.get(), pData, sizeof(unsigned char) * width * height * channels);
            }
            else
            {
                memset(m_pData.get(), 0, sizeof(unsigned char) * width * height * channels);
            }
        }
        else
        {
            BasicLog::out(BasicLog::kWarn, "Invalid data when modifying image.");
        }
    }

    unsigned int BasicImage::width() const
    {
        return m_width;
    }

    unsigned int BasicImage::height() const
    {
        return m_height;
    }

    unsigned char BasicImage::channels() const
    {
        return m_channels;
    }

    const unsigned char* BasicImage::data() const
    {
        return m_pData.get();
    }

    bool BasicImage::valid() const
    {
        return m_width > 0 && m_height > 0 && m_channels > 0 && m_pData != nullptr;
    }

    BasicImage::BasicImage(const BasicImage& other)
    {
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;
        if (other.valid())
        {
            m_pData.reset(new unsigned char[m_width * m_height * m_channels]);
            memcpy(m_pData.get(), other.data(), sizeof(unsigned char) * m_width * m_height * m_channels);
        }
    }

    BasicImage& BasicImage::operator=(const BasicImage& other)
    {
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;
        if (other.valid())
        {
            m_pData.reset(new unsigned char[m_width * m_height * m_channels]);
            memcpy(m_pData.get(), other.data(), sizeof(unsigned char) * m_width * m_height * m_channels);
        }
        return *this;
    }

} // namespace FX
