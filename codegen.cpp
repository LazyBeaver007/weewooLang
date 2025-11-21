#include "codegen.h"
#include "ast.h"

std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;
std::map<std::string, VariableInfo> NamedValues;

llvm::Value* LogErrorV(const char* Str) {
    fprintf(stderr, "Codegen Error: %s\n", Str);
    return nullptr;
}

llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* TheFunction,
    const std::string& VarName, llvm::Type* Type) {

    // Default to double type if not specified
    if (!Type) {
        Type = llvm::Type::getDoubleTy(*TheContext);
    }

    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
        TheFunction->getEntryBlock().begin());

    return TmpB.CreateAlloca(Type, nullptr, VarName);
}

llvm::Value* ExprAST::codegen() {
    return LogErrorV("codegen() not implemented for this AST node");
}

llvm::Value* NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(m_Val));
}

llvm::Value* VariableExprAST::codegen() {
    // Look up the variable in our map
    auto it = NamedValues.find(m_Name);
    if (it == NamedValues.end()) {
        return LogErrorV("Unknown variable name.");
    }

    VariableInfo& varInfo = it->second;

    // Load the value with the correct type
    return Builder->CreateLoad(varInfo.Type, varInfo.Alloca, m_Name.c_str());
}

llvm::Value* BinaryExprAST::codegen() {
    llvm::Value* L = m_LHS->codegen();
    llvm::Value* R = m_RHS->codegen();
    if (!L || !R)
        return nullptr;

    switch (m_Op) {
    case '+':
        return Builder->CreateFAdd(L, R, "addtmp");
    case '-':
        return Builder->CreateFSub(L, R, "subtmp");
    case '*':
        return Builder->CreateFMul(L, R, "multmp");
    case '/':
        return Builder->CreateFDiv(L, R, "divtmp");
    case '<':
        L = Builder->CreateFCmpULT(L, R, "cmptmp");
        return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
    case '>':
        L = Builder->CreateFCmpUGT(L, R, "cmptmp");
        return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
    default:
        return LogErrorV("invalid binary operator");
    }
}

llvm::Value* CallExprAST::codegen() {
    llvm::Function* CalleeF = TheModule->getFunction(m_Callee);
    if (!CalleeF)
        return LogErrorV("Unknown function referenced.");

    if (CalleeF->arg_size() != m_Args.size())
        return LogErrorV("Incorrect # arguments passed.");

    std::vector<llvm::Value*> ArgsV;
    for (unsigned i = 0, e = m_Args.size(); i != e; ++i) {
        ArgsV.push_back(m_Args[i]->codegen());
        if (!ArgsV.back())
            return nullptr;
    }

    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Value* StringExprAST::codegen() {
    // Create a global constant string
    return Builder->CreateGlobalStringPtr(m_Value);
}

/// BoolExprAST codegen  
llvm::Value* BoolExprAST::codegen() {
    // Represent bool as 1.0 (true) or 0.0 (false) to match existing number system
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(m_Value ? 1.0 : 0.0));
}

llvm::Value* PrintAST::codegen() {
    llvm::Value* Val = m_Expr->codegen();
    if (!Val) return nullptr;

    if (Val->getType()->isPointerTy()) {
        // It's a string - call print_string
        llvm::Function* PrintF = TheModule->getFunction("print_string");
        if (!PrintF) {
            // Define print_string function
            llvm::FunctionType* FT = llvm::FunctionType::get(
                llvm::Type::getDoubleTy(*TheContext),
                { llvm::PointerType::get(*TheContext, 0) }, // char* parameter 
                false
            );
            PrintF = llvm::Function::Create(
                FT,
                llvm::Function::ExternalLinkage,
                "print_string",
                TheModule.get()
            );
        }
        return Builder->CreateCall(PrintF, { Val }, "printstrcall");
    }
    else {
        // It's a number/bool - call existing print_double
        llvm::Function* PrintF = TheModule->getFunction("print_double");
        if (!PrintF) {
            std::vector<llvm::Type*> ArgTypes(1, llvm::Type::getDoubleTy(*TheContext));
            llvm::FunctionType* FT = llvm::FunctionType::get(
                llvm::Type::getDoubleTy(*TheContext),
                ArgTypes,
                false
            );
            PrintF = llvm::Function::Create(
                FT,
                llvm::Function::ExternalLinkage,
                "print_double",
                TheModule.get()
            );
        }
        return Builder->CreateCall(PrintF, { Val }, "printcall");
    }
}

llvm::Value* VarDeclAST::codegen() {
    // 1. Get the function we're currently in.
    llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

    // 2. Generate code for the initializer expression.
    llvm::Value* InitVal = m_InitExpr->codegen();
    if (!InitVal) return nullptr;

    // 3. Determine the type based on the initializer
    llvm::Type* VarType = InitVal->getType();

    // 4. Create an 'alloca' for the variable with the correct type
    llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, m_Name, VarType);

    // 5. 'store' the initial value into the new variable.
    Builder->CreateStore(InitVal, Alloca);

    // 6. Save the 'alloca' and type in our variable map.
    NamedValues[m_Name] = VariableInfo(Alloca, VarType);

    // A 'wee' declaration can be an expression, returning the init value.
    return InitVal;
}

llvm::Value* AssignmentAST::codegen() {
    // 1. Generate the code for the new value.
    llvm::Value* NewVal = m_RHS->codegen();
    if (!NewVal) return nullptr;

    // 2. Look up the variable's info
    auto it = NamedValues.find(m_Name);
    if (it == NamedValues.end()) {
        return LogErrorV("Unknown variable referenced in assignment.");
    }

    VariableInfo& varInfo = it->second;

    // 3. 'store' the new value into the variable.
    Builder->CreateStore(NewVal, varInfo.Alloca);

    // An assignment expression returns the new value.
    return NewVal;
}

llvm::Value* IfExprAST::codegen() {
    llvm::Value* CondV = m_Cond->codegen();
    if (!CondV) return nullptr;

    CondV = Builder->CreateFCmpONE(
        CondV, llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)), "ifcond");

    llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

    llvm::BasicBlock* ThenBB = llvm::BasicBlock::Create(*TheContext, "then", TheFunction);
    llvm::BasicBlock* ElseBB = llvm::BasicBlock::Create(*TheContext, "else");
    llvm::BasicBlock* MergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");

    Builder->CreateCondBr(CondV, ThenBB, ElseBB);

    Builder->SetInsertPoint(ThenBB);
    llvm::Value* ThenV = m_Then->codegen();
    if (!ThenV) return nullptr;
    Builder->CreateBr(MergeBB);
    ThenBB = Builder->GetInsertBlock();

    TheFunction->insert(TheFunction->end(), ElseBB);
    Builder->SetInsertPoint(ElseBB);

    llvm::Value* ElseV;
    if (m_Else) {
        ElseV = m_Else->codegen();
        if (!ElseV) return nullptr;
    }
    else {
        ElseV = llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0));
    }

    Builder->CreateBr(MergeBB);
    ElseBB = Builder->GetInsertBlock();

    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder->SetInsertPoint(MergeBB);

    llvm::PHINode* PN = Builder->CreatePHI(llvm::Type::getDoubleTy(*TheContext), 2, "iftmp");
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);

    return PN;
}

llvm::Value* ReturnExprAST::codegen() {
    llvm::Value* RetVal = m_Expr->codegen();
    if (!RetVal) return nullptr;

    Builder->CreateRet(RetVal);
    return RetVal;
}

llvm::Value* BlockAST::codegen() {
    llvm::Value* LastV = nullptr;
    for (const auto& Stmt : m_Statements) {
        LastV = Stmt->codegen();
        if (!LastV) return nullptr;
    }
    return LastV;
}

llvm::Function* PrototypeAST::codegen() {
    std::vector<llvm::Type*> Doubles(m_Args.size(), llvm::Type::getDoubleTy(*TheContext));

    llvm::FunctionType* FT = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext),
        Doubles,
        false);

    llvm::Function* F = llvm::Function::Create(
        FT,
        llvm::Function::ExternalLinkage,
        m_Name,
        TheModule.get());

    unsigned Idx = 0;
    for (auto& Arg : F->args())
        Arg.setName(m_Args[Idx++]);

    return F;
}

llvm::Function* FunctionAST::codegen() {
    llvm::Function* TheFunction = TheModule->getFunction(m_Proto->getName());
    if (!TheFunction)
        TheFunction = m_Proto->codegen();

    if (!TheFunction)
        return nullptr;

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    NamedValues.clear();
    for (auto& Arg : TheFunction->args()) {
        // Create alloca for each argument
        llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, std::string(Arg.getName()));

        // Store the argument value in the alloca
        Builder->CreateStore(&Arg, Alloca);

        // Save in NamedValues
        NamedValues[std::string(Arg.getName())] = VariableInfo(Alloca, Arg.getType());
    }

    if (llvm::Value* RetVal = m_Body->codegen()) {
        // If there's no terminator instruction, create a return
        if (Builder->GetInsertBlock()->getTerminator() == nullptr) {
            Builder->CreateRet(RetVal);
        }

        llvm::verifyFunction(*TheFunction);
        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}

void CodeGenerator::Generate(FunctionAST& ast) {
    ast.codegen();
}

void CodeGenerator::Dump() {
    TheModule->print(llvm::errs(), nullptr);
}