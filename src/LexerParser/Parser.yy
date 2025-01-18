/*
 * Created by Renatus Madrigal on 12/26/2024
 */

// TODO: check if it is necessary to use glr instead of lalr1
// %skeleton "glr2.cc"
%skeleton "lalr1.cc"
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
#include "AST/TypeNodeImpl.h"
#include "AST/TypeNode.h"
#include "AST/ASTNode.h"

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
    LPAREN "("
    RPAREN ")"
    LBRACKET 
    RBRACKET "]"
    LBRACE "{"
    RBRACE "}"
    COMMA ","
    SEMICOLON ";"
    COLON ":"
    COND "?"

%token
    PLUS "+"
    MINUS "-"
    TIMES "*"
    DIVIDE "/"
    MODULO "%"
    BITAND "&"
    BITOR "|"
    BITXOR "^"
    BITNOT "~"
    // TODO: flex may not be able to distinguish between ">>" in template and ">>" in bitshift
    LSHIFT "<<"
    RSHIFT ">>"
    AND "&&"
    OR "||"
    NOT "!"
    EQ "=="
    NE "!="
    LESS "<"
    GREATER ">"
    LEQ "<="
    GEQ ">="
    INC "++"
    DEC "--"
    ASSIGN "="
    ADDASSIGN "+="
    SUBASSIGN "-="
    MULASSIGN "*="
    DIVASSIGN "/="
    MODASSIGN "%="
    ANDASSIGN "&="
    ORASSIGN "|="
    XORASSIGN "^="
    LSHIFTASSIGN "<<="
    RSHIFTASSIGN ">>="
    MEMBER "."
    POINTER "->"
    

%token
    IF "if"
    ELSE "else"
    WHILE "while"
    FOR "for"
    RETURN "return"
    BREAK "break"
    CONTINUE "continue"
    STRUCT "struct"
    USING "using"
    STATIC_CAST "static_cast"
    CONST_CAST "const_cast"
    REINTERPRET_CAST "reinterpret_cast"

%token <std::string> IDENTIFIER "identifier"
%token <std::string> TYPENAME "typename" // TODO: Merge TYPENAME token with IDENTIFIER token
%token <std::string> INT "int"
%token <std::string> HEX_INT "hex_int"
%token <std::string> OCT_INT "oct_int"
%token <std::string> BIN_INT "bin_int"
%token <std::string> FLOAT "float"
%token <std::string> BOOL "bool" // TODO: bool
%token <std::string> CHAR "const_char"
%token <std::string> STRING "const_string"

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
%nterm <AST::FuncDefNode::ParamsElem> param;
%nterm <std::vector<AST::FuncDefNode::ParamsElem>> params;
%nterm <AST::FuncDefPtr> func_def;
%nterm <AST::StructDefNode::FieldsElem> struct_field;
%nterm <std::vector<AST::StructDefNode::FieldsElem>> struct_fields;
%nterm <AST::StructDefPtr> struct_def;
%nterm <AST::AliasDefPtr> alias_def;

%nterm <AST::StmtNodePtr> stmt;
%nterm <std::vector<AST::StmtNodePtr>> stmts;

// Type
%nterm <AST::SimpleTypePtr> simple_type;
%nterm <AST::FuncTypePtr> func_type;
%nterm <AST::ComplexTypePtr> complex_type;

%nterm <AST::TypeNodePtr> type;
%nterm <std::vector<AST::TypeNodePtr>> types;
/* %nterm <std::vector<AST::ASTNodePtr>> types_and_exprs; */

// Expr
%nterm <AST::ConstExprPtr> const_expr;
%nterm <AST::VariablePtr> variable;
%nterm <AST::BinaryPtr> binary;
%nterm <AST::UnaryPtr> unary;
%nterm <AST::MultiaryPtr> multiary;
%nterm <std::vector<AST::ExprNodePtr>> comma_expr;
%nterm <AST::CastPtr> cast;
%nterm <AST::ConditionPtr> condition;

%nterm <AST::ExprNodePtr> expr;

%printer { yyo << $$; } <*>;

%%

%start unit;
unit: 
  stmts { driver.result = driver.allocNode<AST::ASTRootNode>($1); }

block:
  "{" stmts "}" { $$ = driver.allocNode<AST::BlockNode>($2); }

if:
  "if" "(" expr ")" stmt { $$ = driver.allocNode<AST::IfNode>($3, $5, nullptr); }
| "if" "(" expr ")" stmt "else" stmt { $$ = driver.allocNode<AST::IfNode>($3, $5, $7); }

while:
  "while" "(" expr ")" stmt { $$ = driver.allocNode<AST::WhileNode>($3, $5); }

// TODO: Support variable_def in for stmt
for:
  "for" "(" expr ";" expr ";" expr ")" stmt { $$ = driver.allocNode<AST::ForNode>($3, $5, $7, $9); }

return:
  "return" expr ";" { $$ = driver.allocNode<AST::ReturnNode>($2); }
| "return" ";" { $$ = driver.allocNode<AST::ReturnNode>(nullptr); }

break: "break" ";" { $$ = driver.allocNode<AST::BreakNode>(); }
continue: "continue" ";" { $$ = driver.allocNode<AST::ContinueNode>(); }

// TODO: Multiple variable_def in a single line
variable_def:
  type "identifier" ";" { $$ = driver.allocNode<AST::VariableDefNode>($1, $2, nullptr); }
| type "identifier" "=" expr ";" { $$ = driver.allocNode<AST::VariableDefNode>($1, $2, $4); }

// TODO: Reduce-Reduce conflict
param: type "identifier" { $$ = driver.allocNode<AST::FuncDefNode::ParamsElemNode>($1, $2); }
params:
  param { $$ = {$1}; }
| params "," param { $$ = std::move($1); $$.emplace_back($3); }

// TODO: separate declaration and definition
func_def:
  type "identifier" "(" params ")" block { $$ = driver.allocNode<AST::FuncDefNode>($1, $2, $4, $6); }
| type "identifier" "(" ")" block { $$ = driver.allocNode<AST::FuncDefNode>($1, $2, $5); }

expr_stmt: 
  expr ";" { $$ = driver.allocNode<AST::ExprStmtNode>($1); }

struct_field: type "identifier" ";" { $$ = driver.allocNode<AST::StructDefNode::FieldsElemNode>($1, $2, nullptr); }
struct_fields:
  struct_field { $$ = {$1}; }
| struct_fields struct_field { $$ = std::move($1); $$.emplace_back($2); }

struct_def:
  "struct" "typename" "{" struct_fields "}" ";" { $$ = driver.allocNode<AST::StructDefNode>($2, $4); }
| "struct" "typename" "{" "}" ";" { $$ = driver.allocNode<AST::StructDefNode>($2); }

alias_def:
  "using" "typename" "=" type ";" { $$ = driver.allocNode<AST::AliasDefNode>($2, $4); }

stmt:
  block { $$ = $1; }
| if { $$ = $1; }
| while { $$ = $1; }
| for { $$ = $1; }
| return { $$ = $1; }
| break { $$ = $1; }
| continue { $$ = $1; }
| variable_def { $$ = $1; }
| func_def { $$ = $1; }
| struct_def { $$ = $1; }
| alias_def { $$ = $1; }
| expr_stmt { $$ = $1; }

stmts:
  stmt { $$ = {$1}; }
| stmts stmt { $$ = std::move($1); $$.emplace_back($2); };

simple_type: "typename" { $$ = driver.allocNode<AST::SimpleTypeNode>($1); }

func_type:
  type "(" ")" { $$ = driver.allocNode<AST::FuncTypeNode>($1); }
| type "(" types ")" { $$ = driver.allocNode<AST::FuncTypeNode>($1, $3); }

// TODO: Support expr as template argument
complex_type:
  "typename" "<" types ">"  { $$ = driver.allocNode<AST::ComplexTypeNode>($1, $3); }

type:
  simple_type { $$ = $1; }
| func_type { $$ = $1; }
| complex_type { $$ = $1; }

types:
  type { $$ = {$1}; }
| types "," type { $$ = std::move($1); $$.emplace_back($3); }

/* types_and_exprs:
  type { $$ = {$1}; }
| expr { $$ = {$1}; }
| types_and_exprs "," type { $$ = std::move($1); $$.emplace_back($3); }
| types_and_exprs "," expr { $$ = std::move($1); $$.emplace_back($3); } */

const_expr:
  "int" { $$ = driver.allocNode<AST::ConstExprNode>($1, AST::ConstExprType::Int); }
| "hex_int" { $$ = driver.allocNode<AST::ConstExprNode>($1, AST::ConstExprType::HexInt); }
| "oct_int" { $$ = driver.allocNode<AST::ConstExprNode>($1, AST::ConstExprType::OctInt); }
| "bin_int" { $$ = driver.allocNode<AST::ConstExprNode>($1, AST::ConstExprType::BinInt); }
| "float" { $$ = driver.allocNode<AST::ConstExprNode>($1, AST::ConstExprType::Float); }
| "const_string" { $$ = driver.allocNode<AST::ConstExprNode>($1, AST::ConstExprType::String); }
| "const_char" { $$ = driver.allocNode<AST::ConstExprNode>($1, AST::ConstExprType::Char); }
| "bool" { $$ = driver.allocNode<AST::ConstExprNode>($1, AST::ConstExprType::Bool); }

variable: "identifier" { $$ = driver.allocNode<AST::VariableNode>($1); }

binary:
  expr "+" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Add, $3); }
| expr "-" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Sub, $3); }
| expr "*" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Mul, $3); }
| expr "/" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Div, $3); }
| expr "%" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Mod, $3); }
| expr "&" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitAnd, $3); }
| expr "|" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitOr, $3); }
| expr "^" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitXor, $3); }
| expr "<<" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitLShift, $3); }
| expr ">>" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitRShift, $3); }
| expr "&&" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::And, $3); }
| expr "||" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Or, $3); }
| expr "==" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Eq, $3); }
| expr "!=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Ne, $3); }
| expr "<" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Less, $3); }
| expr ">" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Greater, $3); }
| expr "<=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Leq, $3); }
| expr ">=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Geq, $3); }
| expr "=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Assign, $3); }
| expr "+=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::AddAssign, $3); }
| expr "-=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::SubAssign, $3); }
| expr "*=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::MulAssign, $3); }
| expr "/=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::DivAssign, $3); }
| expr "%=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::ModAssign, $3); }
| expr "&=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitAndAssign, $3); }
| expr "|=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitOrAssign, $3); }
| expr "^=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitXorAssign, $3); }
| expr "<<=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitLShiftAssign, $3); }
| expr ">>=" expr { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::BitRShiftAssign, $3); }
// Note that member and ptr_member only allow "identifier" as the right operand
| expr "." "identifier" { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::Member, driver.allocNode<AST::VariableNode>($3)); }
| expr "->" "identifier" { $$ = driver.allocNode<AST::BinaryNode>($1, AST::BinaryOp::PtrMember, driver.allocNode<AST::VariableNode>($3)); }

unary:
  "+" expr { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::Positive, $2); } %prec UPLUS
| "-" expr { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::Negative, $2); } %prec UMINUS
| "!" expr { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::Not, $2); }
| "~" expr { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::BitNot, $2); }
| "++" expr { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::PreInc, $2); } %prec PREINC
| "--" expr { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::PreDec, $2); } %prec PREDEC
| expr "++" { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::PostInc, $1); } 
| expr "--" { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::PostDec, $1); } 
| "&" expr { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::Addr, $2); } %prec ADDR
| "*" expr { $$ = driver.allocNode<AST::UnaryNode>(AST::UnaryOp::Deref, $2); } %prec DEREF

comma_expr:
  expr { $$ = {$1}; }
| comma_expr "," expr { $$ = std::move($1); $$.emplace_back($3); }

multiary:
  "identifier" "(" ")" { $$ = driver.allocNode<AST::MultiaryNode>(AST::MultiaryOp::FuncCall, $1); }
| "identifier" "(" comma_expr ")" { $$ = driver.allocNode<AST::MultiaryNode>(AST::MultiaryOp::FuncCall, $1, $3); }
| "identifier" "[" "]" { $$ = driver.allocNode<AST::MultiaryNode>(AST::MultiaryOp::Subscript, $1); }
| "identifier" "[" comma_expr "]" { $$ = driver.allocNode<AST::MultiaryNode>(AST::MultiaryOp::Subscript, $1, $3); }

cast:
  "static_cast" "<" type ">" "(" expr ")" { $$ = driver.allocNode<AST::CastNode>(AST::CastType::Static, $3, $6); } 
| "const_cast" "<" type ">" "(" expr ")" { $$ = driver.allocNode<AST::CastNode>(AST::CastType::Const, $3, $6); }
| "reinterpret_cast" "<" type ">" "(" expr ")" { $$ = driver.allocNode<AST::CastNode>(AST::CastType::Reinterpret, $3, $6); }

condition:
  expr "?" expr ":" expr { $$ = driver.allocNode<AST::ConditionNode>($1, $3, $5); }

expr:
  const_expr { $$ = $1; }
| variable { $$ = $1; }
| binary { $$ = $1; }
| unary { $$ = $1; }
| multiary { $$ = $1; }
| cast { $$ = $1; }
| condition { $$ = $1; }

%left ",";
%right "=" "+=" "-=" "*=" "/=" "%=" "&=" "|=" "^=" "<<=" ">>=";
%right "?" ":";
%left "||";
%left "&&";
%left "<" "<=" ">" ">=" "==" "!=";
%left "<<" ">>";
%left "|";
%left "^";
%left "&";
%left "+" "-";
%left "*" "/" "%";
%right UPLUS UMINUS;
%left "++" "--";
%right PREINC PREDEC;
%right ADDR DEREF;
%right "!" "~";
%left "(" ")" "[" "]" "{" "}";
%left "." "->";
%right "if" "else" "while" "for" "return" "break" "continue" "struct" "using" "static_cast" "const_cast" "reinterpret_cast";

%%

void TinyCobalt::LexerParser::yy::parser::error(const location_type& l, const std::string& m){
    std::cerr << l << ": " << m << std::endl;
}
