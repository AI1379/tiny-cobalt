/*
* Created by Renatus Madrigal on 12/26/2024
*/

%skeleton "glr2.cc"
%require "3.8"
%define api.namespace { TinyCobalt::LexerParser::yy }
%header

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
#include <string>
#include "AST/ExprNodeImpl.h"
#include "AST/ExprNode.h"
#include "AST/ASTNode.h"
namespace TinyCobalt::LexerParser {
    class YaccDriver;
}
}

%param {
    TinyCobalt::LexerParser::YaccDriver& driver
}

%locations

%define parse.trace
%define parse.error detailed

%code {
#include "LexerParser/YaccDriver.h"

#define yylex driver.lexer->yylex

// namespace AST = TinyCobalt::AST;
using namespace TinyCobalt;
}

%define api.token.prefix {Token_}
%token 
    ASSIGN "="
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
%token <char> CONST_CHAR "const_char"
%nterm <AST::ExprNodePtr> exp

%printer { yyo << $$; } <*>;

%%

%start unit;
unit: exp { driver.result = $1; };

assignments:
  %empty                 {}
| assignments assignment {};

assignment:
  "identifier" "=" exp { driver.variables[$1] = $3; };

%left "+" "-";
%left "*" "/" "%";

exp:
  "number"
| "identifier"  { $$ = driver.variables[$1]; }
| exp "+" exp   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Add, $1, $3); }
| exp "-" exp   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Sub, $1, $3); }
| exp "*" exp   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Mul, $1, $3); }
| exp "/" exp   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Div, $1, $3); }
| exp "%" exp   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Mod, $1, $3); }
| "(" exp ")"   { $$ = $2; }

%%

void TinyCobalt::LexerParser::yy::parser::error(const location_type& l, const std::string& m){
    std::cerr << l << ": " << m << std::endl;
}
