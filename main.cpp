#include "parser.h"
#include "codegen.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>
#include <fstream>
#include <sstream>

extern "C" double print_double(double d) {
    std::cout << "WeeWoo Output: " << d << std::endl;
    return d;
}

int main(int argc, char** argv) {
   
    if (argc != 2) {
        std::cerr << "Usage: weewoo_compiler <filename.weewoo>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return 1;
    }

   
    std::stringstream ss;
    ss << inFile.rdbuf();
    std::string sourceCode = ss.str();

   
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

   
    std::cout << "<--- Parsing Program --->" << std::endl;
    Lexer lex(sourceCode);
    Parser parser(lex);
    parser.RunParse();
    std::cout << "<--- Done Parsing --->" << std::endl;

   
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("WeeWoo JIT", *TheContext);
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

   
    std::cout << "\n<--- Generating LLVM IR --->" << std::endl;
    CodeGenerator codeGen;
   
    for (auto& funcAST : parser.getParsedFunctions()) {
        codeGen.Generate(*funcAST);
    }

   
    std::cout << "\n<--- LLVM IR Dump --->" << std::endl;
    codeGen.Dump();

   
    std::string errStr;
    llvm::EngineBuilder* engineBuilder = new llvm::EngineBuilder(std::move(TheModule));

    engineBuilder->setErrorStr(&errStr);

   
    engineBuilder->setEngineKind(llvm::EngineKind::JIT);

    llvm::ExecutionEngine* EE = engineBuilder->create();
    if (!EE) {
        std::cerr << "Could not create ExecutionEngine: " << errStr << std::endl;
        return 1;
    }

    
    EE->addGlobalMapping("print_double", (uint64_t)print_double);
    EE->finalizeObject();

    
    std::cout << "\n<--- JIT Execution --->" << std::endl;

    // Find the 'main' function
    llvm::Function* mainFunc = EE->FindFunctionNamed("main");
    if (!mainFunc) {
        std::cerr << "Error: Could not find 'main' function to run." << std::endl;
        return 1;
    }

    // Run it
    EE->runFunction(mainFunc, {});

    std::cout << "<--- JIT Finished --->" << std::endl;

    delete EE;
    return 0;
}