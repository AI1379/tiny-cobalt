//
// Created by Renatus Madrigal on 01/27/2025
//

#include "Semantic/DeclMatcher.h"
#include "AST/ASTVisitor.h"
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"
#include "Common/Utility.h"

namespace TinyCobalt::Semantic {

    // TODO: Merge all these functions
    void DeclMatcher::tryAddSymbol(AST::FuncDefPtr ptr) {
        if (current_alias_->getSymbol(ptr->name) != nullptr // NOLINT
            || current_struct_->getSymbol(ptr->name) != nullptr // NOLINT
            || current_variable_->getSymbol(ptr->name) != nullptr) {
            throw std::runtime_error("Symbol " + ptr->name + " already exists");
        }
        current_func_->addSymbol(ptr->name, ptr);
    }

    void DeclMatcher::tryAddSymbol(AST::VariableDefPtr ptr) {
        if (current_alias_->getSymbol(ptr->name) != nullptr // NOLINT
            || current_struct_->getSymbol(ptr->name) != nullptr // NOLINT
            || current_func_->getSymbol(ptr->name) != nullptr) {
            throw std::runtime_error("Symbol " + ptr->name + " already exists");
        }
        current_variable_->addSymbol(ptr->name, ptr);
    }

    void DeclMatcher::tryAddSymbol(AST::AliasDefPtr ptr) {
        if (current_func_->getSymbol(ptr->name) != nullptr // NOLINT
            || current_struct_->getSymbol(ptr->name) != nullptr // NOLINT
            || current_variable_->getSymbol(ptr->name) != nullptr) {
            throw std::runtime_error("Symbol " + ptr->name + " already exists");
        }
        current_alias_->addSymbol(ptr->name, ptr);
    }

    void DeclMatcher::tryAddSymbol(AST::StructDefPtr ptr) {
        if (current_alias_->getSymbol(ptr->name) != nullptr // NOLINT
            || current_func_->getSymbol(ptr->name) != nullptr // NOLINT
            || current_variable_->getSymbol(ptr->name) != nullptr) {
            throw std::runtime_error("Symbol " + ptr->name + " already exists");
        }
        current_struct_->addSymbol(ptr->name, ptr);
    }


    AST::VisitorState DeclMatcher::beforeSubtreeImpl(AST::ASTNodePtr node) {
        auto matcher = Matcher{[&](AST::FuncDefPtr ptr) { tryAddSymbol(ptr); },
                               [&](AST::VariableDefPtr ptr) { tryAddSymbol(ptr); },
                               [&](AST::AliasDefPtr ptr) { tryAddSymbol(ptr); },
                               [&](AST::StructDefPtr ptr) { tryAddSymbol(ptr); },
                               [&](AST::VariablePtr ptr) { ptr->def = current_variable_->getSymbol(ptr->name); },
                               [&](AST::SimpleTypePtr ptr) { ptr->def = findType(ptr->name); }};
        visit(matcher, node);
        return AST::VisitorState::Normal;
    }

    AST::VisitorState DeclMatcher::beforeChildImpl(AST::ASTNodePtr node, AST::ASTNodePtr child) {
        size_t type_hash = proxy_typeid(child).hash_code();
        if (type_hash == typeid(AST::BlockPtr).hash_code()) {
            auto name = kDefaultScopeName;
            if (proxy_typeid(*node).hash_code() == typeid(AST::FuncDefNode).hash_code()) {
                name = proxy_cast<AST::FuncDefPtr>(node)->name;
            }
            pushScope(name);
        }
        return AST::VisitorState::Normal;
    }

    AST::VisitorState DeclMatcher::afterChildImpl(AST::ASTNodePtr node, AST::ASTNodePtr child) {
        size_t type_hash = proxy_typeid(node).hash_code();
        if (type_hash == typeid(AST::BlockPtr).hash_code()) {
            popScope();
        }
        return AST::VisitorState::Normal;
    }

    AST::SimpleTypeNode::TypeDefPtr DeclMatcher::findType(const std::string &name) {
        if (auto alias = current_alias_->getSymbol(name))
            return alias;
        if (auto struc = current_struct_->getSymbol(name))
            return struc;
        if (auto builtin = AST::BuiltInType::findType(name))
            return builtin;
        // throw std::runtime_error("Type " + name + " not found");
        return nullptr;
    }

    // FIXME: MEMORY LEAK!
    // It may be necessary to refactor the Scope with the type of parent set to std::unique_ptr<Scope>.
    void DeclMatcher::pushScope(const std::string &name) {
        current_alias_ = std::make_unique<AliasScope>(std::move(current_alias_), name);
        current_func_ = std::make_unique<FuncScope>(std::move(current_func_), name);
        current_variable_ = std::make_unique<VariableScope>(std::move(current_variable_), name);
        current_struct_ = std::make_unique<StructScope>(std::move(current_struct_), name);
    }

    void DeclMatcher::popScope() {
        current_alias_ = current_alias_->getParent();
        current_func_ = current_func_->getParent();
        current_variable_ = current_variable_->getParent();
        current_struct_ = current_struct_->getParent();
    }
} // namespace TinyCobalt::Semantic