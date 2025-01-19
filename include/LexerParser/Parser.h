//
// Created by Renatus Madrigal on 12/26/2024
//

#ifndef TINY_COBALT_INCLUDE_LEXERPARSER_PARSER_H_
#define TINY_COBALT_INCLUDE_LEXERPARSER_PARSER_H_

#include <iostream>
#include <utility>
#include "AST/AST.h"
#include "LexerParser/Location.h"

namespace TinyCobalt::LexerParser {
    // TODO: use concept to restrict Driver type.
    // We use template wrapper to allow the parser to be used with different driver classes.
    template<typename Driver>
    class BaseParser {
    public:
        BaseParser() { driver = new Driver(); }
        template<typename... Args>
        BaseParser(Args &&...args) {
            driver = new Driver(std::forward<Args>(args)...);
        }
        ~BaseParser() { delete driver; }

        // Return an error code because bison uses it.
        int parse() { return driver->parse(); }

        BaseParser<Driver> &switchInput(std::istream *is) {
            driver->switchInput(is);
            return *this;
        }
        BaseParser<Driver> &switchOutput(std::ostream *os) {
            driver->switchOutput(os);
            return *this;
        }

        AST::ASTRootPtr result() { return driver->result; }

    private:
        Driver *driver;
    };

    // TODO: make YaccLexer and YaccDriver private.

    // Forward declaration of YaccLexer.
    class YaccLexer;

    class YaccDriver {
    public:
        YaccDriver();
        YaccDriver(std::istream *is, std::ostream *os);

        void switchInput(std::istream *is) { this->is = is; }
        void switchOutput(std::ostream *os) { this->os = os; }

        AST::ASTRootPtr result;

        // Start parsing.
        int parse();

        YaccLexer *lexer;

        // Handling the scanner.
        void scan_begin();
        void scan_end();
        // The token's location used by the scanner.
        Location location;

        // For customize allocation
        template<typename T, typename... Args>
        auto allocNode(Args &&...args) {
            return std::make_shared<T>(std::forward<Args>(args)...);
        }

    private:
        // The name of the file being parsed.
        std::string file;
        std::istream *is;
        std::ostream *os;
        // Whether to generate parser debug traces.
        bool trace_parsing;
        // Whether to generate scanner debug traces.
        bool trace_scanning;
    };

    // Using YaccDriver by default.
    using Parser = BaseParser<YaccDriver>;
} // namespace TinyCobalt::LexerParser

#endif // TINY_COBALT_INCLUDE_LEXERPARSER_PARSER_H_