//
// Created by Renatus Madrigal on 12/26/2024
//

#ifndef TINY_COBALT_INCLUDE_LEXERPARSER_PARSER_H_
#define TINY_COBALT_INCLUDE_LEXERPARSER_PARSER_H_

namespace TinyCobalt::LexerParser {
    // We use CRTP to allow the parser to be used with different driver classes.
    template<typename Driver>
    class BaseParser {
    public:
        BaseParser();
    };

    // Forward declaration of YaccDriver.
    class YaccDriver;

    // Using YaccDriver by default.
    using Parser = BaseParser<YaccDriver>;
} // namespace TinyCobalt::LexerParser

#endif // TINY_COBALT_INCLUDE_LEXERPARSER_PARSER_H_