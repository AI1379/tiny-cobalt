//
// Created by Renatus Madrigal on 12/26/2024
//

#ifndef TINY_COBALT_SRC_LEXERPARSER_YACCDRIVER_H_
#define TINY_COBALT_SRC_LEXERPARSER_YACCDRIVER_H_

#include <map>
#include <string>

#include "Parser.tab.hpp"

namespace TinyCobalt::LexerParser {
    class YaccDriver {
    public:
        YaccDriver();

        std::map<std::string, int> variables;

        int result;

        // Run the parser on file F.  Return 0 on success.
        int parse(const std::string &f);
        // The name of the file being parsed.
        std::string file;
        // Whether to generate parser debug traces.
        bool trace_parsing;

        // Handling the scanner.
        void scan_begin();
        void scan_end();
        // Whether to generate scanner debug traces.
        bool trace_scanning;
        // The token's location used by the scanner.
        location location;
    };


} // namespace TinyCobalt::LexerParser

#define YY_DECL TinyCobalt::LexerParser::parser::symbol_type yylex(TinyCobalt::LexerParser::YaccDriver &driver)

YY_DECL;

#endif // TINY_COBALT_SRC_LEXERPARSER_YACCDRIVER_H_
