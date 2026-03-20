# Raccoon
Raccoon is statically-typed, AOT-compiled, garbage collected language.
## Goal
A language with the option to declare functions, variables and structs, it should also be possible to do basic maths. This will require building a compiler and runtime (mainly for Garbage Collection). The Raccoon language will be compiled into LLVM IR and bundled with the runtime. The GC will be shadow stack based.
## Future Goals
Standard library, classes (or at least structs that will be capable of provide some functionality that classes do), generics.
## Languages, libraries
Compiler:  
languge: C++  
libs: LLVM, ZLIB, Zstd, CLI11 
  
Runtime:  
language: C

## Raccoon Syntax
### Basics
The only allowed statements outside of functions are `let` and `den`, so declaring variables, functions, classes and seting dens (namespaces).  
The program runs from the `main` function. The file extension for raccoon source files is `.trash`.

### Variables & Constants
Variables in Raccoon are **immutable by default** and they must be initialized during declaration. Use the `mut` keyword for reassignable values.  
The syntax for declaring variables is: `let {name} be [modifier keywords] {type} = {expression};`  
example of immutable variable: `let pi be float = 3.14;`  
example of mutable variable: `let age be mut int = 22;`  
  
reassigning variables: `age = age + 1;` 

#### Basic Types
- `int`
- `float`
- `bool`
- `void`

### Functions
Functions in Raccoon are handled same as variables, but their type is signature and return type.  
So the syntax for declaring functions is: `let {name} be [modifier keywords] ({parameter type list}) {return type} = ({parameter name list}) {function body};`  
example of function:  
```raccoon
let add be (int, int) int = (a, b) {
    return a + b;
}
```
functions can also be declared as mutable using the `mut` keyword.

### If-Else statements
example of if-else statement:
```raccoon
if age > 19 {
    print(1);
} else {
    print(0);
}
```

### Dens (Namespaces)
`den` blocks act as scoped namespaces to prevent naming collisions.
```raccoon
den Math {
    let PI be float = 3.14;
}