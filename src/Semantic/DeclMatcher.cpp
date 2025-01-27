//
// Created by Renatus Madrigal on 01/27/2025
//

#include "Semantic/DeclMatcher.h"
#include "AST/ASTVisitor.h"

namespace TinyCobalt::Semantic {
    AST::VisitorState DeclMatcher::beforeSubtree(AST::ASTNodePtr node) {
        // TODO: Use switch to optimize
        if (node->containType<AST::FuncDefNode>()) {
            auto decl = node->cast<AST::FuncDefNode>();
            current_func_->addSymbol(decl->name, decl);
        } else if (node->containType<AST::VariableDefNode>()) {
            auto decl = node->cast<AST::VariableDefNode>();
            current_variable_->addSymbol(decl->name, decl);
        } else if (node->containType<AST::AliasDefNode>()) {
            auto decl = node->cast<AST::AliasDefNode>();
            current_alias_->addSymbol(decl->name, decl);
        } else if (node->containType<AST::StructDefNode>()) {
            auto decl = node->cast<AST::StructDefNode>();
            current_struct_->addSymbol(decl->name, decl);
        } else if (node->containType<AST::VariableNode>()) {
            node->cast<AST::VariableNode>()->def = current_variable_->getSymbol(node->cast<AST::VariableNode>()->name);
        } else if (node->containType<AST::SimpleTypeNode>()) {
            auto type = node->cast<AST::SimpleTypeNode>();
            type->def = findType(type->name);
        }
        return AST::VisitorState::Normal;
    }

    AST::VisitorState DeclMatcher::beforeChild(AST::ASTNodePtr node, AST::ASTNodePtr child) {
        if (child->containType<AST::BlockNode>()) {
            auto name = kDefaultScopeName;
            if (node->containType<AST::FuncDefNode>()) {
                name = node->cast<AST::FuncDefNode>()->name;
            }
            pushScope(name);
        }
        return AST::VisitorState::Normal;
    }

    AST::VisitorState DeclMatcher::afterChild(AST::ASTNodePtr node, AST::ASTNodePtr child) {
        if (child->containType<AST::BlockNode>()) {
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