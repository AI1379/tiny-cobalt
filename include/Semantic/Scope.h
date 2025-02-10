//
// Created by Renatus Madrigal on 01/19/2025
//

#ifndef TINY_COBALT_INCLUDE_SEMANTIC_SCOPE_H_
#define TINY_COBALT_INCLUDE_SEMANTIC_SCOPE_H_

#include <format>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include "AST/AST.h"
#include "Common/Concept.h"
#include "Common/FlatMap.h"

namespace TinyCobalt::Semantic {

    inline constexpr std::string kDefaultScopeName = "<anonymous>";

    /**
     * A class to represent a scope in the program.
     */
    template<typename Key, typename Value>
    class Scope {
        template<typename T>
        using ResultType = std::conditional_t<PointerLike<Value>, T, std::optional<T>>;

    public:
        using ParentPointer = std::unique_ptr<Scope<Key, Value>>;
        Scope(ParentPointer &&parent = nullptr, const std::string &name = kDefaultScopeName) :
            parent_(std::move(parent)), name_(name) {}

        /**
         * Add a symbol to the scope.
         * @param name The name of the symbol.
         * @param node The AST node representing the symbol.
         */
        void addSymbol(const Key &key, const Value &value) {
            if (symbols_.contains(key)) {
                throw std::runtime_error(std::format("Symbol already exists in scope \"{}\"", name_));
            }
            symbols_[key] = value;
        }

        /**
         * Get the symbol with the given name.
         * @param name The name of the symbol.
         * @return The AST node representing the symbol.
         */
        ResultType<Value> getSymbol(const Key &name) {
            auto current = this;
            while (current) {
                if (current->symbols_.contains(name)) {
                    return current->symbols_[name];
                }
                current = current->parent_.get();
            }
            if constexpr (!PointerLike<Value>) {
                return std::nullopt;
            } else {
                return nullptr;
            }
        }

        /**
         * Get the symbol with the given name in the current scope.
         */
        ResultType<Value> getLocalSymbol(const Key &name) {
            if (symbols_.contains(name)) {
                return symbols_[name];
            }
            if constexpr (!PointerLike<Value>) {
                return std::nullopt;
            } else {
                return nullptr;
            }
        }

        /**
         * Get the symbol with a pointer to its scope.
         */
        ResultType<std::pair<Value, Scope *>> getSymbolWithScope(const Key &name) {
            auto current = this;
            while (current) {
                if (current->symbols_.contains(name)) {
                    return std::make_pair(current->symbols_[name], current);
                }
                current = current->parent_.get();
            }
            if constexpr (!PointerLike<Value>) {
                return std::nullopt;
            } else {
                return {nullptr, nullptr};
            }
        }

        /**
         * Get the name of the scope.
         */
        const std::string &getName() const { return name_; }

        /**
         * Get the full name of the scope.
         */
        std::string getFullName() const {
            if (parent_) {
                return std::format("{}::{}", parent_->getFullName(), name_);
            }
            return name_;
        }

        /**
         * Get the parent of this scope.
         */
        ParentPointer getParent() { return std::move(parent_); }

        /**
         * Get a observer pointer to the parent.
         */
        Scope *getObserverParent() const { return parent_.get(); }

        using ContainerType = Common::flat_map<Key, Value>;

    private:
        ParentPointer parent_;
        ContainerType symbols_;
        std::string name_;
    };

} // namespace TinyCobalt::Semantic

#endif // TINY_COBALT_INCLUDE_SEMANTIC_SCOPE_H_