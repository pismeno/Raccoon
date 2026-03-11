#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <stdexcept>

namespace raccoon::compiler {

    struct VarInfo {
        std::string name;
        std::string type;
        bool isImmutable;
        bool isGlobal;
    };

    class VarTable {
    public:
        VarTable();

        void enterScope(const std::string& denName = "");
        void exitScope();

        void define(const std::string& name, const std::string& type, bool isImmutable);
        std::optional<SymbolInfo> lookup(const std::string& name);

        bool isAtGlobalScope() const;
        std::string getCurrentMangledPrefix() const;

    private:
        std::vector<std::map<std::string, SymbolInfo>> scopes;
        std::vector<std::string> activeDens;
    };

} // namespace raccoon