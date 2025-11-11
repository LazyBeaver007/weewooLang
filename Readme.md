# **WeeWoo Language**
https://weewoo-web.vercel.app/
A custom programming language implementation built with LLVM for learning compiler design. Inspired by Expedition 33 and Kaleidoscope.

## **Features**

* **Simple Syntax**: Easy-to-learn language with fun keywords  
* **LLVM-powered**: Full compilation pipeline from source to execution  
* **JIT Compilation**: Immediate execution without separate compilation step  
* **Type System**: Everything is a double-precision floating point number  
* **Standard Library**: Built-in I/O operations

## **Language Syntax**

### **Variables**

wee x \= 10      \# Variable declaration  
x \= x \+ 5       \# Variable assignment  
woo x           \# Print variable

### **Control Flow**

weewoo (x \> 15\) {  
    woo 99  
} woowee {  
    woo \-1  
}

### **Functions**

woowoo main() {  
    woo 42  
    weewoowee 100  
}

woowoo add(a, b) {  
    weewoowee a \+ b  
}

### **Expressions**

a \+ b           \# Addition  
a \- b           \# Subtraction  
a \* b           \# Multiplication  
a / b           \# Division  
a \> b           \# Greater than  
a \< b           \# Less than

### **Complete Example**

woowoo main() {  
    woo 1000  
    wee x \= 10  
    woo x  
    x \= x \+ 20  
    woo x  
    weewoo (x \> 25\) {  
        woo 99  
    } woowee {  
        woo \-1  
    }  
    weewoowee 0  
}

### **Language Keywords**

| Keyword | Purpose | Example |
| :---- | :---- | :---- |
| wee | Variable declaration | wee x \= 5 |
| woo | Print statement | woo x |
| weewoo | If statement | weewoo (x \> 5\) |
| woowee | Else clause | woowee { ... } |
| woowoo | Function definition | woowoo main() |
| weewoowee | Return statement | weewoowee 42 |

<img width="1919" height="996" alt="image" src="https://github.com/user-attachments/assets/6b0f295d-44a1-4747-a923-10e327ee3e7d" />

## **Building and Running**

### **Prerequisites**

* LLVM 18.1.5+  
* CMake 3.20+  
* C++17 compatible compiler

### **Build & Run**

mkdir build && cd build  
cmake ..  
make  
./weewoo\_compiler program.weewoo

## **Project Structure**

weewoo\_compiler/  
├── ast.h           \# Abstract Syntax Tree  
├── lexer.h/cpp     \# Lexical analysis  
├── parser.h/cpp    \# Syntax analysis  
├── codegen.h/cpp   \# LLVM IR generation  
├── main.cpp        \# Driver program  
└── CMakeLists.txt  \# Build configuration

## **Current Limitations**

* All values are 64-bit floating point numbers  
* No string support  
* No loops  
* No modules/imports  
* Basic error reporting
