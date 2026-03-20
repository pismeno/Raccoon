#include "../../include/ast/VarTable.hpp"
#include <sstream>

namespace raccoon::compiler::ast {

    VarTable::VarTable() {
        enterScope();
    }

    void VarTable::enterScope(const std::string& denName) {
        scopes.push_back(std::map<std::string, VarInfo>());
        if (!denName.empty()) {
            activeDens.push_back(denName);
        }
    }

    void VarTable::exitScope() {
        if (scopes.size() > 1) {
            scopes.pop_back();
        }
    }

    void VarTable::define(const std::string& name, const std::shared_ptr<Type> type, bool isMutable) {
        VarInfo info;
        info.name = name;
        info.type = type;
        info.isMutable = isMutable;
        info.isGlobal = (scopes.size() == 2);

        scopes.back()[name] = info;
    }

    void VarTable::define(const std::string& name, VarInfo info) {
        scopes.back()[name] = info;
    }

    std::optional<VarInfo> VarTable::lookup(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        return std::nullopt;
    }

    bool VarTable::isAtGlobalScope() const {
        return scopes.size() <= 2;
    }

    std::string VarTable::getCurrentMangledPrefix() const {
        std::stringstream ss;
        for (const auto& den : activeDens) {
            ss << den << "::";
        }
        return ss.str();
    }

} // namespace raccoon::compiler::ast