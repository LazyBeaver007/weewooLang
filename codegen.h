#ifndef WEEWOO_CODEGEN_H
#define WEEWOO_CODEGEN_H

#include <map>
#include <memory>
#include <string>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Instructions.h" // Required for AllocaInst

struct VariableInfo {
    llvm::AllocaInst* Alloca;
    llvm::Type* Type;

    // Constructor to make initialization easier
    VariableInfo(llvm::AllocaInst* alloca = nullptr, llvm::Type* type = nullptr)
        : Alloca(alloca), Type(type) {
    }
};

class FunctionAST;

extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;
extern std::map<std::string, VariableInfo> NamedValues;

llvm::Value* LogErrorV(const char* Str);

llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* TheFunction,
    const std::string& VarName, llvm::Type* Type = nullptr);

// Add CodeGenerator class definition
class CodeGenerator {
public:
    void Generate(FunctionAST& ast);
    void Dump();
};

#endif //WEEWOO_CODEGEN_H