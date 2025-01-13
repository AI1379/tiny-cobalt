//
// Created by Renatus Madrigal on 12/26/2024
//

#ifndef TINY_COBALT_SRC_LEXERPARSER_YACCDRIVER_H_
#define TINY_COBALT_SRC_LEXERPARSER_YACCDRIVER_H_

#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <proxy.h>
#include <stack>
#include <string>

#include "AST/ASTNode.h"
#include "AST/ExprNodeImpl.h"
#include "Parser.tab.hpp"
#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

namespace TinyCobalt::LexerParser {
    class YaccDriver;

    class YaccLexer : public yyFlexLexer {
    public:
        yy::parser::symbol_type yylex(YaccDriver &driver);
    };

    class YaccDriver {
    public:
        YaccDriver();

        std::map<std::string, AST::ExprNodePtr> variables;

        AST::ASTRootNodePtr result;

        // Start parsing.
        int parse();
        
        YaccLexer *lexer;

        // Handling the scanner.
        void scan_begin();
        void scan_end();
        // The token's location used by the scanner.
        yy::location location;

        template<typename T, typename... Args>
        auto allocNode(Args &&...args) {
            return std::make_shared<T>(std::forward<Args>(args)...);
        }

        AST::ASTNodePtr getCurrent() const { return stack_.top(); }

    private:
        // The name of the file being parsed.
        std::string file;
        std::istream *is;
        std::ostream *os;
        // Whether to generate parser debug traces.
        bool trace_parsing;
        // Whether to generate scanner debug traces.
        bool trace_scanning;

        AST::ASTNodePtr ast_;
        std::stack<AST::ASTNodePtr> stack_;
    };


} // namespace TinyCobalt::LexerParser

#endif // TINY_COBALT_SRC_LEXERPARSER_YACCDRIVER_H_
