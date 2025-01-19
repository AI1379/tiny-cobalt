//
// Created by Renatus Madrigal on 01/19/2025
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTROOTNODE_H_
#define TINY_COBALT_INCLUDE_AST_ASTROOTNODE_H_

#include "AST/ASTNode.h"
#include "AST/StmtNode.h"

namespace TinyCobalt::AST {

    struct ASTRootNode : public EnableThisPointer<ASTRootNode> {
        std::vector<StmtNodePtr> children;
        explicit ASTRootNode(std::vector<StmtNodePtr> children) : children(std::move(children)) {}
        ASTNodeGen traverse() {
            for (const auto &child: children) {
                co_yield child;
            }
        }
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "ASTRoot";
            json["children"] = Common::JSON::array();
            for (const auto &child: children) {
                auto child_json = child->toJSON();
                json["children"].push_back(child_json);
            }
            return json;
        }
    };
    using ASTRootPtr = std::shared_ptr<ASTRootNode>;

    static_assert(ASTNodePtrConcept<ASTRootPtr>, "ASTNodePtrConcept<ASTRootPtr> is not satisfied.");

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_ASTROOTNODE_H_