# TODO

## Features

- [x] Dumper
- [x] AST
- [x] Change the basis of ASTNodes from std::variant to pro::proxy
- [x] Lexer and Parser with bison/yacc
- [x] Implement a matcher for proxy types.
- [ ] Const qualifier for type.
- [ ] Better syntax error report.
- [ ] Modularize the implementation of Lexer and Parser.
- [ ] Implement ASTBuilder to build AST.
- [ ] Check if declarations of AST nodes should be private.
- [ ] Support function and operator overload.
- [ ] Introduce C++20 module to enhance compile performance.
- [ ] Separate implementation from header.

## Optimistic

- [x] Check the performance of `std::variant`, `std::visit` and `std::shared_ptr`.
- [x] Implement `flat_map`.