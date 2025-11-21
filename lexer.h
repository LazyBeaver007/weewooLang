

#ifndef WEEWOO_LEXER_H
#define WEEWOO_LEXER_H

#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

enum Token {
    TOKEN_EOF = -1,
    // Keywords
    TOKEN_WEE = -2,         // var
    TOKEN_WOO = -3,         // print
    TOKEN_WEEWOO = -4,       // if
    TOKEN_WOOWEE = -5,    // else
    TOKEN_WEEWEE = -6,     // while
    TOKEN_WOOWOO = -7,     // def
    TOKEN_WEEWOOWEE = -8, // return

    // Literals & Identifiers
    TOKEN_IDENTIFIER = -9,
    TOKEN_NUMBER = -10,

    //other
    TOKEN_STRING = -11,
    TOKEN_BOOL = -12,
    TOKEN_TRUE = -13,
    TOKEN_FALSE = -14,

};


class Lexer {
    private:
    std::string m_Source;
    int m_CurrentIndex = 0;

   
    std::string m_IdentifierStr;
    double m_NumVal = 0;
    char peekNextChar();
    //holds custo keyworks
    std::map<std::string, Token> m_KeywordMap;
    char getNextChar(); 
    std::string m_StringVal;

    public:

        Lexer(const std::string& source);

        int getNextToken();

        //getters for toekn value
        std::string getIdentifierStr() const {return m_IdentifierStr;}
        double getNumVal() const{return m_NumVal;}
        std::string getStringVal() const { return m_StringVal; }
};

#endif 
