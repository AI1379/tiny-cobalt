//
// Created by Renatus Madrigal on 02/05/2025
//

#include "AST/ASTNodeDecl.h"

namespace TinyCobalt::AST {

    struct ComplexTypeNode::Visitor {
        ASTNodePtr operator()(const TypeNodePtr &type) const { return type; }
        ASTNodePtr operator()(const ConstExprPtr &expr) const { return expr; }
    };

    ASTNodeGen ComplexTypeNode::traverse() {
        for (auto &arg : templateArgs) {
            co_yield std::visit(Visitor{}, arg);
        }
    }

    Common::JSON ComplexTypeNode::toJSON() const {
        Common::JSON json;
        json["type"] = "ComplexType";
        json["template_name"] = templateName;
        json["template_args"] = Common::JSON::array();
        for (const auto &arg : templateArgs) {
            json["template_args"].push_back(std::visit(Visitor{}, arg)->toJSON());
        }
        return json;
    }

} // namespace TinyCobalt::AST