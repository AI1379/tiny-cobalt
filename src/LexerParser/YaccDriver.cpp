//
// Created by Renatus Madrigal on 01/12/2025
//

#include "LexerParser/YaccDriver.h"
#include <cassert>
#include "Parser.tab.hpp"

namespace TinyCobalt::LexerParser {
    YaccDriver::YaccDriver() = default;
    YaccDriver::YaccDriver(std::istream *is, std::ostream *os) : is(is), os(os) {}

    int YaccDriver::parse() {
        assert(this->is);
        this->scan_begin();

        yy::parser parser(*this);
        parser.set_debug_level(trace_parsing);
        int res = parser.parse();

        this->scan_end();
        return res;
    }
} // namespace TinyCobalt::LexerParser