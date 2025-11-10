#include "codegen.h"
#include "ast.h"


std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;
std::map<std::string, llvm::AllocaInst*> NamedValues; 


llvm::Value* LogErrorV(const char* Str) {
    fprintf(stderr, "Codegen Error: %s\n", Str);
    return nullptr;
}


llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* TheFunction,
    const std::string& VarName) {
    
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
        TheFunction->getEntryBlock().begin());

   
    return TmpB.CreateAlloca(llvm::Type::getDoubleTy(*TheContext), nullptr, VarName);
}



llvm::Value* ExprAST::codegen() {
 
    return LogErrorV("codegen() not implemented for this AST node");
}


llvm::Value* NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(m_Val));
}


llvm::Value* VariableExprAST::codegen() {
   
    llvm::AllocaInst* Alloca = NamedValues[m_Name];
    if (!Alloca)
        return LogErrorV("Unknown variable name.");

   
    return Builder->CreateLoad(llvm::Type::getDoubleTy(*TheContext), Alloca, m_Name.c_str());
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


llvm::Value* PrintAST::codegen() {
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

    llvm::Value* Val = m_Expr->codegen();
    if (!Val) return nullptr;

    return Builder->CreateCall(PrintF, { Val }, "printcall");
}


llvm::Value* VarDeclAST::codegen() {
   
    llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

   
    llvm::Value* InitVal = m_InitExpr->codegen();
    if (!InitVal) return nullptr;

   
    llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, m_Name);

   
    Builder->CreateStore(InitVal, Alloca);

   
    NamedValues[m_Name] = Alloca;

   
    return InitVal;
}


llvm::Value* AssignmentAST::codegen() {
   
    llvm::Value* NewVal = m_RHS->codegen();
    if (!NewVal) return nullptr;

    
    llvm::AllocaInst* Alloca = NamedValues[m_Name];
    if (!Alloca)
        return LogErrorV("Unknown variable referenced in assignment.");

   
    Builder->CreateStore(NewVal, Alloca);

  
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
      
        llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, std::string(Arg.getName()));
        
        Builder->CreateStore(&Arg, Alloca);
        
        NamedValues[std::string(Arg.getName())] = Alloca;
    }

    if (llvm::Value* RetVal = m_Body->codegen()) {

      
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