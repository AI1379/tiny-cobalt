//
// Created by Renatus Madrigal on 01/19/2025
//

#ifndef TINY_COBALT_INCLUDE_SEMANTIC_SCOPE_H_
#define TINY_COBALT_INCLUDE_SEMANTIC_SCOPE_H_

#include <map>
#include <string>
#include "AST/AST.h"
#include "AST/ASTNode.h"

namespace TinyCobalt::Semantic {
    /**
     * A class to represent a scope in the program.
     */
    class Scope {
    public:
        Scope(Scope *parent) : parent_(parent) {}

        /**
         * Add a symbol to the scope.
         * @param name The name of the symbol.
         * @param node The AST node representing the symbol.
         */
        void addSymbol(const std::string &name, AST::ASTNodePtr node);

        /**
         * Get the symbol with the given name.
         * @param name The name of the symbol.
         * @return The AST node representing the symbol.
         */
        AST::ASTNodePtr getSymbol(const std::string &name);

    private:
#if __cpp_lib_flat_map >= 202207L
        using ContainerType = std::flat_map<std::string, AST::ASTNodePtr>;
#else
        using ContainerType = std::map<std::string, AST::ASTNodePtr>;
#endif

        Scope *parent_;
        // TODO: implement a flat map to store the symbols.
        ContainerType symbols_;
    };

} // namespace TinyCobalt::Semantic

#endif // TINY_COBALT_INCLUDE_SEMANTIC_SCOPE_H_