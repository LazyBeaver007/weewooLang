#include "lexer.h"
#include <cctype>
#include <string>
#include <iostream> 

Lexer::Lexer(const std::string& source) : m_Source(source), m_CurrentIndex(0), m_NumVal(0) {
    //populate keyword map
    m_KeywordMap["wee"] = TOKEN_WEE;
    m_KeywordMap["woo"] = TOKEN_WOO;
    m_KeywordMap["weewoo"] = TOKEN_WEEWOO;
    m_KeywordMap["woowee"] = TOKEN_WOOWEE;
    m_KeywordMap["weewee"] = TOKEN_WEEWEE;
    m_KeywordMap["woowoo"] = TOKEN_WOOWOO;
    m_KeywordMap["weewoowee"] = TOKEN_WEEWOOWEE;
}

//helper for getting next char and move index
char Lexer::getNextChar() {
    if (m_CurrentIndex < m_Source.length()) {
        return m_Source[m_CurrentIndex++];
    }
    return '\0'; // End of file
}

// Peek at next character without consuming it
char Lexer::peekNextChar() {
    if (m_CurrentIndex < m_Source.length()) {
        return m_Source[m_CurrentIndex];
    }
    return '\0';
}

//main
// In lexer.cpp, update the getNextToken() method:

int Lexer::getNextToken() {
    static char LastChar = ' ';

    //skipping whitespaces
    while (std::isspace(LastChar)) {
        LastChar = getNextChar();
    }

    //comments 
    if (LastChar == '#') {
        do {
            LastChar = getNextChar();
        } while (LastChar != '\0' && LastChar != '\n' && LastChar != '\r');

        //if at not eof recursively call getnexttoken for getting next one after comment
        if (LastChar != '\0') {
            return getNextToken();
        }
    }

    // Check for EOF
    if (LastChar == '\0') {
        return TOKEN_EOF;
    }

    // Handle negative numbers and subtraction operator
    if (LastChar == '-') {
        // Peek at the next character to see if it's part of a number
        char nextChar = peekNextChar();
        if (isdigit(nextChar) || nextChar == '.') {
            // It's a negative number, not subtraction
            std::string NumStr;
            NumStr += LastChar; // Add the '-' sign
            LastChar = getNextChar(); // Move to the digit

            bool hasDecimal = false;
            do {
                NumStr += LastChar;
                LastChar = getNextChar();
                if (LastChar == '.' && !hasDecimal) {
                    hasDecimal = true;
                }
                else if (LastChar == '.' && hasDecimal) {
                    break; // Second decimal point, invalid number
                }
            } while (isdigit(LastChar) || LastChar == '.');

            m_NumVal = strtod(NumStr.c_str(), nullptr);
            return TOKEN_NUMBER;
        }
        else {
            // It's a subtraction operator
            int ThisChar = LastChar;
            LastChar = getNextChar();
            return ThisChar;
        }
    }

    //identifiers and keywords
    if (isalpha(LastChar)) { // Starts with a letter
        m_IdentifierStr = LastChar;
        while (isalnum(peekNextChar()) || peekNextChar() == '_') {
            LastChar = getNextChar();
            m_IdentifierStr += LastChar;
        }
        LastChar = getNextChar(); // Get next char for next token

        // Check if this identifier is a known keyword
        if (m_KeywordMap.count(m_IdentifierStr)) {
            return m_KeywordMap[m_IdentifierStr];
        }
        return TOKEN_IDENTIFIER;
    }

    //for positive numbers 
    if (isdigit(LastChar) || LastChar == '.') {
        std::string NumStr;
        bool hasDecimal = (LastChar == '.');

        do {
            NumStr += LastChar;
            LastChar = getNextChar();
            if (LastChar == '.' && !hasDecimal) {
                hasDecimal = true;
            }
            else if (LastChar == '.' && hasDecimal) {
                break; // Second decimal point, invalid number
            }
        } while (isdigit(LastChar) || LastChar == '.');

        m_NumVal = strtod(NumStr.c_str(), nullptr);
        return TOKEN_NUMBER;
    }


    int ThisChar = LastChar;
    LastChar = getNextChar(); 

    return ThisChar;
}