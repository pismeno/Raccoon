# Raccoon
Raccoon is statically-typed, AOT-compiled, garbage collected language.

# Goal
A language with the option to declare functions, variables and structs, it should also be possible to do basic maths. This will require building a compiler and runtime (mainly for Garbage Collection). The Raccoon language will be compiled into LLVM IR and bundled with the runtime. The GC will be shadow stack based.
# In future
Standard library, classes (or at least structs that will be capable of provide some functionality that classes do), generics.
# Languages, libraries
Compiler:  
languge: C++  
libs: LLVM, ZLIB, Zstd  
  
Runtime:  
language: C
