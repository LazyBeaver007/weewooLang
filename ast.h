#ifndef WEEWOO_AST_H
#define WEEWOO_AST_H

#include <string>
#include <vector>
#include <memory>

namespace llvm {
    class Value;
    class Function; 
}


class ExprAST {
public:
    virtual ~ExprAST() = default;
    virtual llvm::Value* codegen() = 0;
};


class NumberExprAST : public ExprAST {
    double m_Val;
public:
    NumberExprAST(double val) : m_Val(val) {}
    llvm::Value* codegen() override;
};



// String literal expression
class StringExprAST : public ExprAST {
    std::string m_Value;
public:
    StringExprAST(const std::string& value) : m_Value(value) {}
    llvm::Value* codegen() override;
};

// Boolean literal expression  
class BoolExprAST : public ExprAST {
    bool m_Value;
public:
    BoolExprAST(bool value) : m_Value(value) {}
    llvm::Value* codegen() override;
};

class VariableExprAST : public ExprAST {
    std::string m_Name;
public:
    VariableExprAST(const std::string& name) : m_Name(name) {}
    llvm::Value* codegen() override;
};


class BinaryExprAST : public ExprAST {
    char m_Op;
    std::unique_ptr<ExprAST> m_LHS, m_RHS;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs,
        std::unique_ptr<ExprAST> rhs)
        : m_Op(op), m_LHS(std::move(lhs)), m_RHS(std::move(rhs)) {
    }
    llvm::Value* codegen() override;
};


class VarDeclAST : public ExprAST {
    std::string m_Name;
    std::unique_ptr<ExprAST> m_InitExpr; 
public:
    VarDeclAST(const std::string& name, std::unique_ptr<ExprAST> initExpr)
        : m_Name(name), m_InitExpr(std::move(initExpr)) {
    }
    llvm::Value* codegen() override;
};


class AssignmentAST : public ExprAST {
    std::string m_Name;
    std::unique_ptr<ExprAST> m_RHS;
public:
    AssignmentAST(const std::string& name, std::unique_ptr<ExprAST> rhs)
        : m_Name(name), m_RHS(std::move(rhs)) {
    }
    llvm::Value* codegen() override;
};


class PrintAST : public ExprAST {
    std::unique_ptr<ExprAST> m_Expr;
public:
    PrintAST(std::unique_ptr<ExprAST> expr) : m_Expr(std::move(expr)) {}
    llvm::Value* codegen() override;
};



class CallExprAST : public ExprAST {
    std::string m_Callee;
    std::vector<std::unique_ptr<ExprAST>> m_Args;
public:
    CallExprAST(const std::string& callee,
        std::vector<std::unique_ptr<ExprAST>> args)
        : m_Callee(callee), m_Args(std::move(args)) {
    }
    llvm::Value* codegen() override;
};


class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> m_Cond;
    std::unique_ptr<ExprAST> m_Then;
    std::unique_ptr<ExprAST> m_Else; 
public:
    IfExprAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> then,
        std::unique_ptr<ExprAST> elseExpr)
        : m_Cond(std::move(cond)), m_Then(std::move(then)), m_Else(std::move(elseExpr)) {
    }
    llvm::Value* codegen() override;
};


class ReturnExprAST : public ExprAST {
    std::unique_ptr<ExprAST> m_Expr; 
public:
    ReturnExprAST(std::unique_ptr<ExprAST> expr) : m_Expr(std::move(expr)) {}
    llvm::Value* codegen() override;
};


class BlockAST : public ExprAST {
    std::vector<std::unique_ptr<ExprAST>> m_Statements;
public:
    BlockAST(std::vector<std::unique_ptr<ExprAST>> statements)
        : m_Statements(std::move(statements)) {
    }
    llvm::Value* codegen() override;
};


class PrototypeAST {
    std::string m_Name;
    std::vector<std::string> m_Args;
public:
    PrototypeAST(const std::string& name, std::vector<std::string> args)
        : m_Name(name), m_Args(std::move(args)) {
    }

    const std::string& getName() const { return m_Name; }
    llvm::Function* codegen();
};


class FunctionAST {
    std::unique_ptr<PrototypeAST> m_Proto;
    std::unique_ptr<ExprAST> m_Body;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto,
        std::unique_ptr<ExprAST> body)
        : m_Proto(std::move(proto)), m_Body(std::move(body)) {
    }

    const PrototypeAST* getProto() const { return m_Proto.get(); }
    llvm::Function* codegen();
};



#endif //WEEWOO_AST_H