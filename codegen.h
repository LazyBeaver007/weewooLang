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


class FunctionAST;

extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;


extern std::map<std::string, llvm::AllocaInst*> NamedValues;


llvm::Value* LogErrorV(const char* Str);


llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* TheFunction,
    const std::string& VarName);


class CodeGenerator {
public:
   

   
    void Dump();
};


#endif //WEEWOO_CODEGEN_H