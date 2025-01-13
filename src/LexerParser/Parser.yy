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
#include "AST/ExprNodeImpl.h"
#include "AST/ExprNode.h"
#include "AST/StmtNodeImpl.h"
#include "AST/StmtNode.h"
#include "AST/ASTNode.h"

// namespace AST = TinyCobalt::AST;
using namespace TinyCobalt;

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
    LBRAKET "["
    RBRAKET "]"
    LBRACE "{"
    RBRACE "}"
    COMMA ","
    SEMICOLON ";"
;

%token <std::string> IDENTIFIER "identifier"
%token <AST::ExprNodePtr> NUMBER "number"
%token <char> CONST_CHAR "const_char"
%nterm <AST::ExprNodePtr> expr
%nterm <std::vector<AST::ExprNodePtr>> exprs;
%nterm <AST::AssignPtr> assignment;
%nterm <AST::StmtNodePtr> stmt;
%nterm <std::vector<AST::StmtNodePtr>> stmts;
%nterm <AST::BlockPtr> block;

%printer { yyo << $$; } <*>;

%%

%start unit;
unit: 
  stmts { driver.result = std::make_shared<AST::ASTRootNode>($1); }

block:
  "{" stmts "}" { $$ = driver.allocNode<AST::BlockNode>($2); }

assignment:
  "identifier" "=" expr { driver.variables[$1] = $3; };

expr:
  "number"
| "identifier"  { $$ = driver.variables[$1]; }
| expr "+" expr   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Add, $1, $3); }
| expr "-" expr   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Sub, $1, $3); }
| expr "*" expr   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Mul, $1, $3); }
| expr "/" expr   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Div, $1, $3); }
| expr "%" expr   { $$ = driver.allocNode<AST::BinaryNode>(AST::BinaryOp::Mod, $1, $3); }
| "(" expr ")"   { $$ = $2; }

exprs:
  expr { $$ = {$1}; }
| exprs expr { $$ = std::move($1); $$.emplace_back($2); };

stmt:
  block { $$ = $1; };
| expr ";" { $$ = driver.allocNode<AST::ExprStmtNode>($1); }

stmts:
  stmt { $$ = {$1}; }
| stmts stmt { $$ = std::move($1); $$.emplace_back($2); };

%left "+" "-";
%left "*" "/" "%";

%%

void TinyCobalt::LexerParser::yy::parser::error(const location_type& l, const std::string& m){
    std::cerr << l << ": " << m << std::endl;
}
