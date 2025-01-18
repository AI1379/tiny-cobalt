/*
 * Created by Renatus Madrigal on 1/12/2025
 */

%{
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <cstring> // strerror
# include <string>
# include "LexerParser/YaccDriver.h"
# include "Parser.tab.hpp"

#undef YY_DECL
#define YY_DECL TinyCobalt::LexerParser::yy::parser::symbol_type \
                TinyCobalt::LexerParser::YaccLexer::yylex(TinyCobalt::LexerParser::YaccDriver &driver)
%}

%{
// These are copied from bison's example.
#if defined __clang__
# define CLANG_VERSION (__clang_major__ * 100 + __clang_minor__)
#endif

// Clang and ICC like to pretend they are GCC.
#if defined __GNUC__ && !defined __clang__ && !defined __ICC
# define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#endif

// Pacify warnings in yy_init_buffer (observed with Flex 2.6.4)
// and GCC 6.4.0, 7.3.0 with -O3.
#if defined GCC_VERSION && 600 <= GCC_VERSION
# pragma GCC diagnostic ignored "-Wnull-dereference"
#endif

// This example uses Flex's C back end, yet compiles it as C++.
// So expect warnings about C style casts and NULL.
#if defined CLANG_VERSION && 500 <= CLANG_VERSION
# pragma clang diagnostic ignored "-Wold-style-cast"
# pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#elif defined GCC_VERSION && 407 <= GCC_VERSION
# pragma GCC diagnostic ignored "-Wold-style-cast"
# pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#define FLEX_VERSION (YY_FLEX_MAJOR_VERSION * 100 + YY_FLEX_MINOR_VERSION)

// Old versions of Flex (2.5.35) generate an incomplete documentation comment.
//
//  In file included from src/scan-code-c.c:3:
//  src/scan-code.c:2198:21: error: empty paragraph passed to '@param' command
//        [-Werror,-Wdocumentation]
//   * @param line_number
//     ~~~~~~~~~~~~~~~~~^
//  1 error generated.
#if FLEX_VERSION < 206 && defined CLANG_VERSION
# pragma clang diagnostic ignored "-Wdocumentation"
#endif

// Old versions of Flex (2.5.35) use 'register'.  Warnings introduced in
// GCC 7 and Clang 6.
#if FLEX_VERSION < 206
# if defined CLANG_VERSION && 600 <= CLANG_VERSION
#  pragma clang diagnostic ignored "-Wdeprecated-register"
# elif defined GCC_VERSION && 700 <= GCC_VERSION
#  pragma GCC diagnostic ignored "-Wregister"
# endif
#endif

#if FLEX_VERSION < 206
# if defined CLANG_VERSION
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wdocumentation"
#  pragma clang diagnostic ignored "-Wshorten-64-to-32"
#  pragma clang diagnostic ignored "-Wsign-conversion"
# elif defined GCC_VERSION
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
# endif
#endif

// Flex 2.6.4, GCC 9
// warning: useless cast to type 'int' [-Wuseless-cast]
// 1361 |   YY_CURRENT_BUFFER_LVALUE->yy_buf_size = (int) (new_size - 2);
//      |                                                 ^
#if defined GCC_VERSION && 900 <= GCC_VERSION
# pragma GCC diagnostic ignored "-Wuseless-cast"
#endif                                                         

%}

%option c++ noyywrap nounput noinput batch debug

%option yyclass="TinyCobalt::LexerParser::YaccLexer"

%{
  namespace yy = TinyCobalt::LexerParser::yy;

  // A number symbol corresponding to the value in S.
  yy::parser::symbol_type
  make_NUMBER (const std::string &s, const yy::parser::location_type& loc);
%}

id      [a-zA-Z_][a-zA-Z_0-9]*
int     [0-9]+
hex_int 0x[0-9a-fA-F]+
oct_int 0o[0-7]+
bin_int 0b[01]+
float   [0-9]*\.[0-9]+
char    \'[^\']\'
string  \"[^\"]*\"
bool    true|false
/* TODO: Currently we use _t suffix to distinguish type names from other identifiers. */
type    uint|int|float|char|bool|void|[a-zA-Z][a-zA-Z_0-9]*_t
blank   [ \t\r]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}
%%
%{
  // A handy shortcut to the location held by the driver.
  yy::location& loc = driver.location;
  // Code run each time yylex is called.
  loc.step ();
%}
{blank}+   loc.step();
\n+        loc.lines(yyleng); loc.step();

"-"        return yy::parser::make_MINUS(loc);
"+"        return yy::parser::make_PLUS(loc);
"*"        return yy::parser::make_TIMES(loc);
"/"        return yy::parser::make_DIVIDE(loc);
"%"        return yy::parser::make_MODULO(loc);
"&"        return yy::parser::make_BITAND(loc);
"|"        return yy::parser::make_BITOR(loc);
"^"        return yy::parser::make_BITXOR(loc);
"~"        return yy::parser::make_BITNOT(loc);
"<<"       return yy::parser::make_LSHIFT(loc);
">>"       return yy::parser::make_RSHIFT(loc);
"&&"       return yy::parser::make_AND(loc);
"||"       return yy::parser::make_OR(loc);
"!"        return yy::parser::make_NOT(loc);
"=="       return yy::parser::make_EQ(loc);
"!="       return yy::parser::make_NE(loc);
"<"        return yy::parser::make_LESS(loc);
"<="       return yy::parser::make_LEQ(loc);
">"        return yy::parser::make_GREATER(loc);
">="       return yy::parser::make_GEQ(loc);
"++"       return yy::parser::make_INC(loc);
"--"       return yy::parser::make_DEC(loc);
"="        return yy::parser::make_ASSIGN(loc);
"+="       return yy::parser::make_ADDASSIGN(loc);
"-="       return yy::parser::make_SUBASSIGN(loc);
"*="       return yy::parser::make_MULASSIGN(loc);
"/="       return yy::parser::make_DIVASSIGN(loc);
"%="       return yy::parser::make_MODASSIGN(loc);
"&="       return yy::parser::make_ANDASSIGN(loc);
"|="       return yy::parser::make_ORASSIGN(loc);
"^="       return yy::parser::make_XORASSIGN(loc);
"<<="      return yy::parser::make_LSHIFTASSIGN(loc);
">>="      return yy::parser::make_RSHIFTASSIGN(loc);
"."        return yy::parser::make_MEMBER(loc);
"->"       return yy::parser::make_POINTER(loc);

"("        return yy::parser::make_LPAREN(loc);
")"        return yy::parser::make_RPAREN(loc);
"["        return yy::parser::make_LBRACKET(loc);
"]"        return yy::parser::make_RBRACKET(loc);
"{"        return yy::parser::make_LBRACE(loc);
"}"        return yy::parser::make_RBRACE(loc);
","        return yy::parser::make_COMMA(loc);
";"        return yy::parser::make_SEMICOLON(loc);
":"        return yy::parser::make_COLON(loc);
"?"        return yy::parser::make_COND(loc);

"if"       return yy::parser::make_IF(loc);
"else"     return yy::parser::make_ELSE(loc);
"while"    return yy::parser::make_WHILE(loc);
"for"      return yy::parser::make_FOR(loc);
"return"   return yy::parser::make_RETURN(loc);
"break"    return yy::parser::make_BREAK(loc);
"continue" return yy::parser::make_CONTINUE(loc);
"struct"   return yy::parser::make_STRUCT(loc);
"using"    return yy::parser::make_USING(loc);

"static_cast"      return yy::parser::make_STATIC_CAST(loc);
"const_cast"       return yy::parser::make_CONST_CAST(loc);
"reinterpret_cast" return yy::parser::make_REINTERPRET_CAST(loc);

{int}      return yy::parser::make_INT(yytext, loc);
{hex_int}  return yy::parser::make_HEX_INT(yytext, loc);
{oct_int}  return yy::parser::make_OCT_INT(yytext, loc);
{bin_int}  return yy::parser::make_BIN_INT(yytext, loc);
{float}    return yy::parser::make_FLOAT(yytext, loc);
{char}     return yy::parser::make_CHAR(yytext, loc);
{string}   return yy::parser::make_STRING(yytext, loc);
{bool}     return yy::parser::make_BOOL(yytext, loc);
{type}     return yy::parser::make_TYPENAME(yytext, loc);
{id}       return yy::parser::make_IDENTIFIER (yytext, loc);
.          {
             throw yy::parser::syntax_error
               (loc, "invalid character: " + std::string(yytext));
}
<<EOF>>    return yy::parser::make_YYEOF (loc);
%%

void TinyCobalt::LexerParser::YaccDriver::scan_begin ()
{
  this->lexer = new YaccLexer;
  this->lexer->set_debug(trace_scanning);
  this->lexer->switch_streams(*(this->is), *(this->os));
}

void TinyCobalt::LexerParser::YaccDriver::scan_end ()
{
  delete this->lexer;
}
