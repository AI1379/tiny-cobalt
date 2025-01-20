//
// Created by Renatus Madrigal on 01/19/2025
//

#include "Semantic/Scope.h"
#include "AST/ASTNode.h"

namespace TinyCobalt::Semantic {
    void Scope::addSymbol(const std::string &name, AST::ASTNodePtr node) {
        // TODO: use custom exceptions.
        if (symbols_.find(name) != symbols_.end()) {
            throw std::runtime_error("Symbol already exists in scope");
        }
        symbols_[name] = node;
    }

    AST::ASTNodePtr Scope::getSymbol(const std::string &name) {
        auto current = this;
        while (current) {
            if (current->symbols_.find(name) != current->symbols_.end()) {
                return current->symbols_[name];
            }
        }
        return nullptr;
    }

} // namespace TinyCobalt::Semantic
