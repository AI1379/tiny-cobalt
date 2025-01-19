//
// Created by Renatus Madrigal on 12/26/2024
//

#ifndef TINY_COBALT_SRC_LEXERPARSER_YACCDRIVER_H_
#define TINY_COBALT_SRC_LEXERPARSER_YACCDRIVER_H_

#include <proxy.h>

#include "LexerParser/Parser.h"
#include "Parser.tab.hpp"
#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

namespace TinyCobalt::LexerParser {
    class YaccLexer : public yyFlexLexer {
    public:
        yy::parser::symbol_type yylex(YaccDriver &driver);
    };

} // namespace TinyCobalt::LexerParser

#endif // TINY_COBALT_SRC_LEXERPARSER_YACCDRIVER_H_
