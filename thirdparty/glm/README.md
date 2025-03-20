![glm](./logo-mini.png)

[OpenGL Mathematics](http://glm.g-truc.net/) (*GLM*) is a header only C++ mathematics library for graphics software based on the [OpenGL Shading Language (GLSL) specifications](https://www.opengl.org/registry/doc/GLSLangSpec.4.50.diff.pdf).

*GLM* provides classes and functions designed and implemented with the same naming conventions and functionality than *GLSL* so that anyone who knows *GLSL*, can use *GLM* as well in C++.

This project isn't limited to *GLSL* features. An extension system, based on the *GLSL* extension conventions, provides extended capabilities: matrix transformations, quaternions, data packing, random numbers, noise, etc...

This library works perfectly with *[OpenGL](https://www.opengl.org)* but it also ensures interoperability with other third party libraries and SDK. It is a good candidate for software rendering (raytracing / rasterisation), image processing, physics simulations and any development context that requires a simple and convenient mathematics library.

*GLM* is written in C++98 but can take advantage of C++11 when supported by the compiler. It is a platform independent library with no dependence and it officially supports the following compilers:
- [*GCC*](http://gcc.gnu.org/) 4.7 and higher
- [*Intel C++ Compose*](https://software.intel.com/en-us/intel-compilers) XE 2013 and higher
- [*Clang*](http://llvm.org/) 3.4 and higher
- [*Apple Clang 6.0*](https://developer.apple.com/library/mac/documentation/CompilerTools/Conceptual/LLVMCompilerOverview/index.html) and higher
- [*Visual C++*](http://www.visualstudio.com/) 2013 and higher
- [*CUDA*](https://developer.nvidia.com/about-cuda) 9.0 and higher (experimental)
- Any C++11 compiler

For more information about *GLM*, please have a look at the [manual](manual.md) and the [API reference documentation](http://glm.g-truc.net/0.9.9/api/modules.html).
The source code and the documentation are licensed under either the [Happy Bunny License (Modified MIT) or the MIT License](manual.md#section0).

Thanks for contributing to the project by [submitting pull requests](https://github.com/g-truc/glm/pulls).

```cpp
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi

glm::mat4 camera(float Translate, glm::vec2 const& Rotate)
{
	glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.f);
	glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Translate));
	View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	return Projection * View * Model;
}
```
