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
    COLON ":"
;

%token
    IF "if"
    ELSE "else"
    WHILE "while"
    FOR "for"
    RETURN "return"
    BREAK "break"
    CONTINUE "continue"

%token <std::string> IDENTIFIER "identifier"
%token <AST::ExprNodePtr> NUMBER "number"
%token <char> CONST_CHAR "const_char"
%nterm <AST::ExprNodePtr> expr
%nterm <std::vector<AST::ExprNodePtr>> exprs;
%nterm <AST::AssignPtr> assignment;

// Stmt
%nterm <AST::IfPtr> if;
%nterm <AST::WhilePtr> while;
%nterm <AST::ForPtr> for;
%nterm <AST::ReturnPtr> return;
%nterm <AST::BlockPtr> block;
%nterm <AST::ExprStmtPtr> expr_stmt;
%nterm <AST::BreakPtr> break;
%nterm <AST::ContinuePtr> continue;
%nterm <AST::VariableDefPtr> variable_def;
%nterm <std::vector<FuncDefNode::ParamsElem>> params;
%nterm <AST::FuncDefPtr> func_def;

%nterm <AST::StmtNodePtr> stmt_without_semicolon;
%nterm <AST::StmtNodePtr> stmt;
%nterm <std::vector<AST::StmtNodePtr>> stmts;

// Type
%nterm <AST::TypeNodePtr> type;

%printer { yyo << $$; } <*>;

%%

%start unit;
unit: 
  stmts { driver.result = std::make_shared<AST::ASTRootNode>($1); }

block:
  "{" stmts "}" { $$ = driver.allocNode<AST::BlockNode>($2); }

assignment:
  "identifier" "=" expr { driver.variables[$1] = $3; };

if:
  "if" "(" expr ")" stmt { $$ = driver.allocNode<AST::IfNode>($3, $5); }

while:
  "while" "(" expr ")" stmt { $$ = driver.allocNode<AST::WhileNode>($3, $5); }

for:
  "for" "(" stmt_without_semicolon ";" expr ";" expr ")" stmt { $$ = driver.allocNode<AST::ForNode>($3, $5, $7, $9); }

return:
  "return" expr ";" { $$ = driver.allocNode<AST::ReturnNode>($2); }
| "return" ";" { $$ = driver.allocNode<AST::ReturnNode>(nullptr); }

break: "break" { $$ = driver.allocNode<AST::BreakNode>(); }
continue: "continue" { $$ = driver.allocNode<AST::ContinueNode>(); }

variable_def:
  type "identifier" { $$ = driver.allocNode<AST::VariableDefNode>($1, $2, nullptr); }
| type "identifier" "=" expr { $$ = driver.allocNode<AST::VariableDefNode>($1, $2, $4); }
// | "identifier" ":" type { $$ = driver.allocNode<AST::VariableDefNode>($2, $1, nullptr); }
// | "identifier" ":" type "=" expr { $$ = driver.allocNode<AST::VariableDefNode>($2, $1, $4); }

// TODO: Reduce-Reduce conflict
params:
  %empty {}
| variable_def { $$ = { $1 } }
| params "," variable_def { $$ = std::move($1); $$.emplace_back($3); }

func_def:
  type "identifier" "(" params ")" block { $$ = driver.allocNode<AST::FuncDefNode>($1, $2, $4, $6); }

// TODO: Reduce-Reduce conflict
expr_stmt: 
  expr { $$ = driver.allocNode<AST::ExprStmtNode>($1); }

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

stmt_without_semicolon:
  block { $$ = $1; };
| expr_stmt { $$ = $1; }

stmt: stmt_without_semicolon ";" { $$ = $1; }

stmts:
  stmt { $$ = {$1}; }
| stmts stmt { $$ = std::move($1); $$.emplace_back($2); };

type:
  "identifier"

%left "+" "-";
%left "*" "/" "%";

%%

void TinyCobalt::LexerParser::yy::parser::error(const location_type& l, const std::string& m){
    std::cerr << l << ": " << m << std::endl;
}
