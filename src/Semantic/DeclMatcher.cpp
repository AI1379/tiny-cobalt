//
// Created by Renatus Madrigal on 01/27/2025
//

#include "Semantic/DeclMatcher.h"
#include "AST/ASTVisitor.h"

namespace TinyCobalt::Semantic {
    AST::VisitorState DeclMatcher::beforeSubtree(AST::ASTNodePtr node) {
        size_t type_hash = proxy_typeid(*node).hash_code();
        if (type_hash == typeid(AST::FuncDefNode).hash_code()) {
            auto &decl = proxy_cast<AST::FuncDefNode &>(*node);
            current_func_->addSymbol(decl.name, decl.shared_from_this());
        } else if (type_hash == typeid(AST::VariableDefNode).hash_code()) {
            auto &decl = proxy_cast<AST::VariableDefNode &>(*node);
            current_variable_->addSymbol(decl.name, decl.shared_from_this());
        } else if (type_hash == typeid(AST::AliasDefNode).hash_code()) {
            auto &decl = proxy_cast<AST::AliasDefNode &>(*node);
            current_alias_->addSymbol(decl.name, decl.shared_from_this());
        } else if (type_hash == typeid(AST::StructDefNode).hash_code()) {
            auto &decl = proxy_cast<AST::StructDefNode &>(*node);
            current_struct_->addSymbol(decl.name, decl.shared_from_this());
        } else if (type_hash == typeid(AST::VariableNode).hash_code()) {
            auto &decl = proxy_cast<AST::VariableNode &>(*node);
            decl.def = current_variable_->getSymbol(decl.name);
        } else if (type_hash == typeid(AST::SimpleTypeNode).hash_code()) {
            auto &type = proxy_cast<AST::SimpleTypeNode &>(*node);
            type.def = findType(type.name);
        }
        return AST::VisitorState::Normal;
    }

    AST::VisitorState DeclMatcher::beforeChild(AST::ASTNodePtr node, AST::ASTNodePtr child) {
        size_t type_hash = proxy_typeid(*node).hash_code();
        if (type_hash == typeid(AST::BlockNode).hash_code()) {
            auto name = kDefaultScopeName;
            if (proxy_typeid(*node).hash_code() == typeid(AST::FuncDefNode).hash_code()) {
                name = proxy_cast<AST::FuncDefNode &>(*node).name;
            }
            pushScope(name);
        }
        return AST::VisitorState::Normal;
    }

    AST::VisitorState DeclMatcher::afterChild(AST::ASTNodePtr node, AST::ASTNodePtr child) {
        size_t type_hash = proxy_typeid(*node).hash_code();
        if (type_hash == typeid(AST::BlockNode).hash_code()) {
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

    void DeclMatcher::pushScope(const std::string &name) {
        current_alias_ = new AliasScope(current_alias_, name);
        current_func_ = new FuncScope(current_func_, name);
        current_variable_ = new VariableScope(current_variable_, name);
        current_struct_ = new StructScope(current_struct_, name);
    }

    void DeclMatcher::popScope() {
        auto alias = current_alias_;
        current_alias_ = alias->getParent();
        delete alias;
        auto func = current_func_;
        current_func_ = func->getParent();
        delete func;
        auto variable = current_variable_;
        current_variable_ = variable->getParent();
        delete variable;
        auto struc = current_struct_;
        current_struct_ = struc->getParent();
        delete struc;
    }
} // namespace TinyCobalt::Semantic