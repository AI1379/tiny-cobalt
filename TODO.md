# TODO

## Features

- [x] Dumper
- [x] AST
- [x] Change the basis of ASTNodes from std::variant to pro::proxy
- [x] Lexer and Parser with bison/yacc
- [ ] Better syntax error report.
- [ ] Modularize the implementation of Lexer and Parser.
- [ ] Implement ASTBuilder to build AST.

## Optimistic

- [ ] Check the performance of `std::variant`, `std::visit` and `std::shared_ptr`