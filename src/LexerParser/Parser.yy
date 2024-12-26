/*
* Created by Renatus Madrigal on 12/26/2024
*/

%skeleton "lalr1.cc"
%require "3.8"
%define api.namespace { TinyCobalt::LexerParser }
%header

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
#include <string>
namespace TinyCobalt::LexerParser{
    class YaccDriver;
}
}

%param {
    TinyCobalt::LexerParser::YaccDriver& driver
}

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
#include "LexerParser/YaccDriver.h"
}

%define api.token.prefix {Token_}
%token 
    ASSIGN ":="
    PLUS "+"
    MINUS "-"
    TIMES "*"
    DIVIDE "/"
    MODULO "%"
    LPAREN "("
    RPAREN ")"
;

%token <std::string> IDENTIFIER "identifier"
%token <int> NUMBER "number"
%nterm <int> exp

%printer { yyo << $$; } <*>;

%%

%start unit;
unit: assignments exp  { driver.result = $2; };

assignments:
  %empty                 {}
| assignments assignment {};

assignment:
  "identifier" ":=" exp { driver.variables[$1] = $3; };

%left "+" "-";
%left "*" "/";
exp:
  "number"
| "identifier"  { $$ = driver.variables[$1]; }
| exp "+" exp   { $$ = $1 + $3; }
| exp "-" exp   { $$ = $1 - $3; }
| exp "*" exp   { $$ = $1 * $3; }
| exp "/" exp   { $$ = $1 / $3; }
| "(" exp ")"   { $$ = $2; }

%%

void TinyCobalt::LexerParser::parser::error(const location_type& l, const std::string& m){
    std::cerr << l << ": " << m << std::endl;
}
