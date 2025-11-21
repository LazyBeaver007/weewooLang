#ifndef WEEWOO_PARSER_H
#define WEEWOO_PARSER_H

#include "lexer.h"
#include "ast.h"
#include <map>
#include <vector> /

class Parser {
private:
    Lexer& m_Lexer;
    int m_CurrentToken;

    /// m_BinOpPrecedence the precedence for each binary operator.
    std::map<char, int> m_BinOpPrecedence;

    /// getNextToken  helper to get the next token from the lexer.
    int getNextToken() {
        return m_CurrentToken = m_Lexer.getNextToken();
    }

    /// Helper function to get the precedence of the current token.
    int getTokenPrecedence();

    /// Error handling helper
    std::unique_ptr<ExprAST> logError(const char* str);
    std::unique_ptr<PrototypeAST> logErrorP(const char* str);

   
    std::unique_ptr<ExprAST> parseNumberExpr();
    std::unique_ptr<ExprAST> parseParenExpr();
    std::unique_ptr<ExprAST> parseIdentifierExpr(); // This is now much smarter
    std::unique_ptr<ExprAST> parseIfExpr();
    std::unique_ptr<ExprAST> parseVarDecl();
    std::unique_ptr<ExprAST> parsePrintStmt();
    std::unique_ptr<ExprAST> parseBlock();
    std::unique_ptr<ExprAST> parseReturnExpr();
    std::unique_ptr<ExprAST> parsePrimary();
    std::unique_ptr<ExprAST>parseStringExpr();
    std::unique_ptr<ExprAST>parseBoolExpr();
    std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs);
    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<PrototypeAST> parsePrototype();
    std::unique_ptr<FunctionAST> parseDefinition();

    //  to hold all the parsed functions 
    std::vector<std::unique_ptr<FunctionAST>> m_ParsedFunctions;

public:
    Parser(Lexer& lexer);

    // API for the Parser 
    void HandleDefinition();

    // Main loop for parsing the whole file
    void RunParse();

    // Getter for the parsed functions
    std::vector<std::unique_ptr<FunctionAST>>& getParsedFunctions() {
        return m_ParsedFunctions;
    }
};

#endif //WEEWOO_PARSER_H