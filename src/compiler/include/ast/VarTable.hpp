#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <stdexcept>
#include "llvm/IR/Value.h"
#include "Type.hpp"

namespace raccoon::compiler::ast {

    struct VarInfo {
        std::string name;
        std::shared_ptr<Type> type;
        bool isMutable;
        bool isGlobal;
        llvm::Value* address = nullptr;
    };

    class VarTable {
    public:
        VarTable();

        void enterScope(const std::string& denName = "");
        void exitScope();

        void define(const std::string& name, const std::shared_ptr<Type>, bool isImmutable);
        void define(const std::string& name, VarInfo info);
        std::optional<VarInfo> lookup(const std::string& name);

        bool isAtGlobalScope() const;
        std::string getCurrentMangledPrefix() const;

    private:
        std::vector<std::map<std::string, VarInfo>> scopes;
        std::vector<std::string> activeDens;
    };

} // namespace raccoon::compiler::ast