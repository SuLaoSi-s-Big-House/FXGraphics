# FXGraphics

### 简介

我的个人图形仓库，用于日常编码练习和OpenGL功能测试。基于glfw3与glad开发，为简单的OpenGL应用提供支持

### 要求

C++ 14+  (2014年发布)

OpenGL 4.3+  (2012年发布)

### 构建项目

推荐使用Git拉取代码，推荐使用CMake (3.12+) 构建项目

如果你满足上述条件，则可以通过克隆的方式获取代码，在Git bash中运行如下指令：

    git clone https://github.com/SuLaoSi-s-Big-House/FXGraphics.git

完成后，在Git bash中调用仓库中的cmake.sh工具（或双击运行），使用cmake构建项目：

    cd ./FXGraphics
    ./cmake.sh

如果你不满足上述条件，也推荐使用其他方法拉取代码和构建项目

### 运行

推荐使用Visual Studio (MSVC) 编译和运行项目

仓库中的FXMain项目会被编译为exe文件，其余仓库会被编译为dll库，因此将FXMain设置为启动项便可以直接运行，同样你可以直接在FXMain中修改代码用于简单的测试

FXMain项目并不重要，如果你是将FXGraphics作为子文件夹加入到你的项目中，或只需要使用编译生成的dll库，则可以删除FXMain项目

所有编译结果会输出到FXGraphics/bin文件夹中

### 注意事项

此项目的只是为了简化OpenGL的绘制流程，并不是完备的图形系统，要求使用者有一定的OpenGL基础（包括基本绘制流程、shader编写、矩阵运算等）

实现中使用了较多的friend class，不要学，正确的做法是将类拆分为接口类与实现类

在release模式下我设置了隐藏控制台，可以在顶级的CMake文件中找到并修改

### 使用的库

- [glfw3](https://www.glfw.org/)
- [glad](https://github.com/Dav1dde/glad)
- [stb](https://github.com/nothings/stb)
- [glm](https://github.com/g-truc/glm) 
