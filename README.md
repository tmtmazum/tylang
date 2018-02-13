# tylang
Welcome to the tylang wiki!

The following is a compiler front-end project for a toy language. For simplicity the final output of this project is [LLVM-IR](https://llvm.org/docs/LangRef.html). This allows us to focus on the high-level language features, leaving the complex problem of optimization/generating machine-specific assembly to the Clang compiler tools.

## Dependencies
- [Clang tools](http://releases.llvm.org/download.html#svn) (for compiling LLVM IR)
   - needs to be installed separately before compiling the project
- [Python 3+](https://www.python.org/downloads/release/python-363/) (for running tests)
   - needs to be installed separately before compiling the project
- [CMake](https://cmake.org/download/) (for building the ty compiler)
   - needs to be installed separately before compiling the project
- Platform-specific C++ compiler (for building the ty compiler) (VS 2015/17 or any other supported by CMake)
- [cppcoretools](https://github.com/tmtmazum/cppcoretools) (header-only library)
   - included as a submodule
   
See Wiki for detailed description --> https://github.com/tmtmazum/tylang/wiki

## Step-wise Compilation Commands

### Tokenizer Step
`tyx.exe tokenize "foo = @() -> { foo() + 5 }"`

Outputs:
```
Tokenizing 'foo = @() -> {foo() + 5}'
id 'foo'
defn '='
param '@'
paren_open '('
paren_close ')'
arrow '->'
brace_open '{'
id 'foo'
paren_open '('
paren_close ')'
plus '+'
num '5'
brace_close '}'
eof '
```

### Parser Step
`tyx.exe parse "foo = @() -> { foo() + 5 }"`

Outputs
```
- FunctionDefnExpr() 
 - ReturnExpr 
  - BinaryOpExpr(+)  
   - FunctionCallExpr(foo) 
   - Int32LiteralExpr(5) 
exported symbols: (none)
```

### LLVM-IR Generation Step
`tyx.exe emit_llvm  "export(foo)    foo = @() -> { 5 }"`

Outputs:
```
define i32 @foo() {
ret i32 5
}
```
