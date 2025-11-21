#include "parser.h"


Parser::Parser(Lexer& lexer) : m_Lexer(lexer) {
    
    m_BinOpPrecedence['<'] = 10;
    m_BinOpPrecedence['>'] = 10;
    m_BinOpPrecedence['+'] = 20;
    m_BinOpPrecedence['-'] = 20;
    m_BinOpPrecedence['*'] = 40;
    m_BinOpPrecedence['/'] = 40;
}



/// stringexpr ::= '"' characters '"'
std::unique_ptr<ExprAST> Parser::parseStringExpr() {
    std::string value = m_Lexer.getStringVal();
    getNextToken(); // consume the string token
    return std::make_unique<StringExprAST>(value);
}

/// boolexpr ::= 'true' | 'false'
std::unique_ptr<ExprAST> Parser::parseBoolExpr() {
    bool value = (m_CurrentToken == TOKEN_TRUE);
    getNextToken(); // consume the bool token
    return std::make_unique<BoolExprAST>(value);
}


std::unique_ptr<ExprAST> Parser::logError(const char* str) {
    fprintf(stderr, "Error: %s\n", str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::logErrorP(const char* str) {
    logError(str);
    return nullptr;
}

int Parser::getTokenPrecedence() {
    if (!isascii(m_CurrentToken))
        return -1;

   
    int prec = m_BinOpPrecedence[m_CurrentToken];
    if (prec <= 0) return -1;
    return prec;
}


std::unique_ptr<ExprAST> Parser::parseNumberExpr() {
    auto result = std::make_unique<NumberExprAST>(m_Lexer.getNumVal());
    getNextToken(); 
    return std::move(result);
}


std::unique_ptr<ExprAST> Parser::parseParenExpr() {
    getNextToken(); 
    auto v = parseExpression();
    if (!v)
        return nullptr;

    if (m_CurrentToken != ')')
        return logError("expected ')'");
    getNextToken(); // eat ')'.
    return v;
}

/// identifierexpr 
///   ::= identifier                       (variable access)
///   ::= identifier '(' expression* ')'     (function call)
///   ::= identifier '=' expression          (variable assignment)
std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() {
    std::string idName = m_Lexer.getIdentifierStr();

    getNextToken(); // eat identifier.

    // Case 1: Variable assignment (x = 5)
    if (m_CurrentToken == '=') {
        getNextToken(); // eat '='.
        auto rhs = parseExpression();
        if (!rhs) return nullptr;
        return std::make_unique<AssignmentAST>(idName, std::move(rhs));
    }

    // Case 2: Function call (my_func(...))
    if (m_CurrentToken == '(') {
        getNextToken(); // eat '('.
        std::vector<std::unique_ptr<ExprAST>> args;
        if (m_CurrentToken != ')') {
            while (true) {
                if (auto arg = parseExpression())
                    args.push_back(std::move(arg));
                else
                    return nullptr;

                if (m_CurrentToken == ')')
                    break;

                if (m_CurrentToken != ',')
                    return logError("Expected ')' or ',' in argument list");
                getNextToken();
            }
        }
        getNextToken(); // Eat the ')'.
        return std::make_unique<CallExprAST>(idName, std::move(args));
    }

    // Case 3: Simple variable access (x)
    return std::make_unique<VariableExprAST>(idName);
}

/// ifexpr ::= 'weewoo' '(' expression ')' block ['woowee' block]
std::unique_ptr<ExprAST> Parser::parseIfExpr() {
    getNextToken(); // eat 'weewoo'.

    if (m_CurrentToken != '(')
        return logError("Expected '(' after 'weewoo'");

    getNextToken(); // eat '('.
    auto cond = parseExpression();
    if (!cond) return nullptr;

    if (m_CurrentToken != ')')
        return logError("Expected ')' in 'weewoo' condition");

    getNextToken(); // eat ')'.

    auto thenBlock = parseBlock();
    if (!thenBlock) return nullptr;

    std::unique_ptr<ExprAST> elseBlock;
    if (m_CurrentToken == TOKEN_WOOWEE) { // 'woowee' (else)
        getNextToken(); // eat 'woowee'
        elseBlock = parseBlock();
        if (!elseBlock) return nullptr;
    }

    return std::make_unique<IfExprAST>(std::move(cond), std::move(thenBlock), std::move(elseBlock));
}

/// varexpr ::= 'wee' identifier '=' expression
std::unique_ptr<ExprAST> Parser::parseVarDecl() {
    getNextToken(); // eat 'wee'.

    if (m_CurrentToken != TOKEN_IDENTIFIER)
        return logError("Expected identifier after 'wee'");

    std::string varName = m_Lexer.getIdentifierStr();
    getNextToken(); // eat identifier.

    if (m_CurrentToken != '=')
        return logError("Expected '=' after variable name");

    getNextToken(); // eat '='.

    auto initExpr = parseExpression();
    if (!initExpr) return nullptr;

    return std::make_unique<VarDeclAST>(varName, std::move(initExpr));
}

/// printexpr ::= 'woo' expression
std::unique_ptr<ExprAST> Parser::parsePrintStmt() {
    getNextToken(); // eat 'woo'.
    auto expr = parseExpression();
    if (!expr) return nullptr;
    return std::make_unique<PrintAST>(std::move(expr));
}

/// returnexpr ::= 'weewoowee' expression
std::unique_ptr<ExprAST> Parser::parseReturnExpr() {
    getNextToken(); // eat 'weewoowee'.

    auto expr = parseExpression();
    if (!expr) return nullptr;

    return std::make_unique<ReturnExprAST>(std::move(expr));
}

/// block ::= '{' (expression | statement)* '}'
std::unique_ptr<ExprAST> Parser::parseBlock() {
    fprintf(stderr, "parseBlock() called, current token: %d\n", m_CurrentToken);

    if (m_CurrentToken != '{') {
        fprintf(stderr, "Expected '{' to start a block, got: %d\n", m_CurrentToken);
        return logError("Expected '{' to start a block");
    }
    getNextToken(); // eat '{'.
    fprintf(stderr, "After eating '{', current token: %d\n", m_CurrentToken);

    std::vector<std::unique_ptr<ExprAST>> statements;

    while (m_CurrentToken != '}' && m_CurrentToken != TOKEN_EOF) {
        fprintf(stderr, "In block, parsing statement. Current token: %d\n", m_CurrentToken);

        std::unique_ptr<ExprAST> stmt;
        switch (m_CurrentToken) {
        case TOKEN_WEE:
            fprintf(stderr, "  Parsing variable declaration\n");
            stmt = parseVarDecl();
            break;
        case TOKEN_WOO:
            fprintf(stderr, "  Parsing print statement\n");
            stmt = parsePrintStmt();
            break;
        case TOKEN_WEEWOO:
            fprintf(stderr, "  Parsing if statement\n");
            stmt = parseIfExpr();
            break;
        case TOKEN_WEEWOOWEE:
            fprintf(stderr, "  Parsing return statement\n");
            stmt = parseReturnExpr();
            break;
        default:
            fprintf(stderr, "  Parsing as expression\n");
            // This will now correctly handle x = 5
            stmt = parseExpression();
            break;
        }

        if (stmt) {
            statements.push_back(std::move(stmt));
            fprintf(stderr, "  Successfully parsed statement\n");
        }
        else {
            fprintf(stderr, "  Failed to parse statement\n");
            // Error occurred, stop parsing block
            return nullptr;
        }
    }

    if (m_CurrentToken != '}') {
        fprintf(stderr, "Expected '}' to end a block, got: %d\n", m_CurrentToken);
        return logError("Expected '}' to end a block");
    }

    getNextToken(); // eat '}'.
    fprintf(stderr, "Successfully parsed block with %zu statements\n", statements.size());

    return std::make_unique<BlockAST>(std::move(statements));
}


/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
///   ::= ifexpr
///   ::= varexpr
///   ::= printexpr
// In parsePrimary() method, add support for unary minus:
std::unique_ptr<ExprAST> Parser::parsePrimary() {
    switch (m_CurrentToken) {
    case TOKEN_IDENTIFIER:
        return parseIdentifierExpr();
    case TOKEN_NUMBER:
        return parseNumberExpr();
    case '(':
        return parseParenExpr();
    case TOKEN_WEEWOO: // 'if'
        return parseIfExpr();
    case TOKEN_WEE: // 'var'
        return parseVarDecl();
    case TOKEN_WOO: // 'print'
        return parsePrintStmt();
    case TOKEN_WEEWOOWEE: // 'return'
        return parseReturnExpr();

    case TOKEN_STRING:
        return parseStringExpr();
    case TOKEN_TRUE:
    case TOKEN_FALSE:
        return parseBoolExpr();

    case '{':
        return parseBlock();
    case '-': // Unary minus
        getNextToken(); // eat '-'
        if (auto operand = parsePrimary()) {
            // Create a binary expression: 0 - operand
            auto zero = std::make_unique<NumberExprAST>(0.0);
            return std::make_unique<BinaryExprAST>('-', std::move(zero), std::move(operand));
        }
        return nullptr;
    default:
        fprintf(stderr, "Unknown token: %d (char: '%c')\n", m_CurrentToken,
            isprint(m_CurrentToken) ? m_CurrentToken : '?');
        return logError("unknown token when expecting an expression");


    
    }
}

/// binoprhs
///   ::= ('+' primary)*
std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int exprPrec,
    std::unique_ptr<ExprAST> lhs) {
    while (true) {
        int tokPrec = getTokenPrecedence();

        if (tokPrec < exprPrec)
            return lhs;

        int binOp = m_CurrentToken;
        getNextToken(); // eat binop

        auto rhs = parsePrimary();
        if (!rhs)
            return nullptr;

        int nextPrec = getTokenPrecedence();
        if (tokPrec < nextPrec) {
            rhs = parseBinOpRHS(tokPrec + 1, std::move(rhs));
            if (!rhs)
                return nullptr;
        }

        lhs = std::make_unique<BinaryExprAST>(binOp, std::move(lhs), std::move(rhs));
    }
}





/// expression
///   ::= primary binoprhs
///
std::unique_ptr<ExprAST> Parser::parseExpression() {
    auto lhs = parsePrimary();
    if (!lhs)
        return nullptr;

    return parseBinOpRHS(0, std::move(lhs));
}

/// prototype
///   ::= id '(' (id (',' id)*)? ')'
std::unique_ptr<PrototypeAST> Parser::parsePrototype() {
    if (m_CurrentToken != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Expected function name, got token: %d\n", m_CurrentToken);
        return logErrorP("Expected function name in prototype");
    }

    std::string fnName = m_Lexer.getIdentifierStr();
    fprintf(stderr, "Parsing function prototype: %s\n", fnName.c_str());
    getNextToken();

    if (m_CurrentToken != '(') {
        fprintf(stderr, "Expected '(' after function name, got token: %d\n", m_CurrentToken);
        return logErrorP("Expected '(' in prototype");
    }

    getNextToken(); // eat '('.

    std::vector<std::string> argNames;
    if (m_CurrentToken != ')') { // Check if we have arguments
        while (true) {
            if (m_CurrentToken != TOKEN_IDENTIFIER) {
                fprintf(stderr, "Expected identifier in argument list, got token: %d\n", m_CurrentToken);
                return logErrorP("Expected identifier in argument list");
            }

            argNames.push_back(m_Lexer.getIdentifierStr());
            fprintf(stderr, "  Argument: %s\n", m_Lexer.getIdentifierStr().c_str());
            getNextToken(); // eat identifier

            if (m_CurrentToken == ')')
                break; // End of list

            if (m_CurrentToken != ',') {
                fprintf(stderr, "Expected ',' or ')' in argument list, got token: %d\n", m_CurrentToken);
                return logErrorP("Expected ')' or ',' in argument list");
            }

            getNextToken(); // eat ','
        }
    }

    // success.
    getNextToken(); // eat ')'.
    fprintf(stderr, "Successfully parsed prototype for: %s with %zu args\n", fnName.c_str(), argNames.size());

    return std::make_unique<PrototypeAST>(fnName, std::move(argNames));
}

/// definition ::= 'woowoo' prototype block
std::unique_ptr<FunctionAST> Parser::parseDefinition() {
    fprintf(stderr, "Starting to parse function definition\n");
    getNextToken(); // eat 'woowoo'.

    auto proto = parsePrototype();
    if (!proto) {
        fprintf(stderr, "Failed to parse prototype\n");
        return nullptr;
    }

    fprintf(stderr, "Successfully parsed prototype, now parsing body...\n");

    if (auto body = parseBlock()) {
        fprintf(stderr, "Successfully parsed function body\n");
        return std::make_unique<FunctionAST>(std::move(proto), std::move(body));
    }

    fprintf(stderr, "Failed to parse function body\n");
    return nullptr;
}


// --- Public API ---

// THIS FUNCTION IS UPDATED
void Parser::HandleDefinition() {
    fprintf(stderr, "Handling function definition...\n");

    if (auto fnAST = parseDefinition()) {
        if (fnAST->getProto()->getName() == "main") {
            fprintf(stderr, "Successfully parsed 'main' function.\n");
        }
        else {
            fprintf(stderr, "Successfully parsed function definition (%s).\n", fnAST->getProto()->getName().c_str());
        }

        // Save the parsed 
        m_ParsedFunctions.push_back(std::move(fnAST));
    }
    else {
        fprintf(stderr, "Failed to parse function definition. Current token: %d\n", m_CurrentToken);
        // Skip tokens untilfind something recognizable for error recovery
        while (m_CurrentToken != TOKEN_EOF &&
            m_CurrentToken != TOKEN_WOOWOO &&
            m_CurrentToken != '}') {
            getNextToken();
        }
    }
}

/// Top-level driver loop
void Parser::RunParse() {
    getNextToken(); // Prime the token stream
    while (m_CurrentToken != TOKEN_EOF) {
        fprintf(stderr, "Top-level token: %d", m_CurrentToken);
        if (m_CurrentToken == TOKEN_IDENTIFIER) {
            fprintf(stderr, " (identifier: %s)", m_Lexer.getIdentifierStr().c_str());
        }
        else if (m_CurrentToken == TOKEN_NUMBER) {
            fprintf(stderr, " (number: %f)", m_Lexer.getNumVal());
        }
        else if (isprint(m_CurrentToken)) {
            fprintf(stderr, " (char: '%c')", m_CurrentToken);
        }
        fprintf(stderr, "\n");

        switch (m_CurrentToken) {
        case TOKEN_WOOWOO:
            HandleDefinition();
            break;
        case TOKEN_EOF:
            break; // End of file
        default:
           
            fprintf(stderr, "Skipping unexpected top-level token\n");
            getNextToken();
            break;
        }
    }
}