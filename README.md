# tylang
Welcome to the tylang wiki!

The following is a compiler front-end project for a toy language. For simplicity the final output of this project is [LLVM-IR](https://llvm.org/docs/LangRef.html). This allows us to focus on the high-level language features, leaving the complex problem of optimization/generating machine-specific assembly to the Clang compiler tools.

## External Dependencies
- [Clang tools](http://releases.llvm.org/download.html#svn) (for compiling LLVM IR)
   - needs to be installed separately before compiling the project
- [CMake](https://cmake.org/download/)
   - needs to be installed separately before compiling the project
- Platform-specific C++ compiler (VS 2015/17 or any other supported by CMake)
- [cppcoretools](https://github.com/tmtmazum/cppcoretools)
   - included as a submodule
- [tinyxml2](https://github.com/leethomason/tinyxml2)
   - included in this repository
