#include "../../include/ast/VarTable.hpp"
#include <sstream>

namespace raccoon::compiler::ast {

    std::vector<std::string> explode(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));

            start = end + delimiter.length();

            end = str.find(delimiter, start);
        }

        tokens.push_back(str.substr(start));

        return tokens;
    }

    size_t countOccurrences(const std::string& str, const std::string& target) {
        if (target.empty()) return 0;

        size_t count = 0;
        size_t pos = str.find(target, 0);

        while (pos != std::string::npos) {
            count++;
            pos = str.find(target, pos + target.length());
        }

        return count;
    }

    VarTable::VarTable() {
        enterScope();
    }

    void VarTable::enterScope() {
        scopes.push_back(std::map<std::string, VarInfo>());
    }

    void VarTable::exitScope() {
        if (scopes.size() > 1) {
            scopes.pop_back();
        }
    }

    void VarTable::enterDens(const std::string name) {
        std::vector<std::string> dens = explode(name, "::");
        for (const auto& den : dens) {
            activeDens.push_back(den);
        }
    }

    void VarTable::exitDens(unsigned int numDens) {
        if (numDens > activeDens.size()) {
            activeDens.clear();
        } else {
            activeDens.resize(activeDens.size() - numDens);
        }
    }

    void VarTable::exitDens(const std::string name) {
        exitDens(countOccurrences(name, "::") + 1);
    }

    void VarTable::define(const std::string& name, const std::shared_ptr<Type> type, bool isMutable) {
        std::string mangledName = isAtGlobalScope() ? getCurrentMangledPrefix() + name : name;

        VarInfo info;
        info.name = mangledName;
        info.type = type;
        info.isMutable = isMutable;
        info.isGlobal = isAtGlobalScope();

        scopes.back()[mangledName] = info;
    }

    void VarTable::define(const std::string& name, VarInfo info) {
        std::string mangledName = isAtGlobalScope() ? getCurrentMangledPrefix() + name : name;
        info.name = mangledName;
        scopes.back()[mangledName] = info;
    }

    std::optional<VarInfo> VarTable::lookup(const std::string& name) {
        std::string mangledLookup = name;
        size_t pos = 0;
        while ((pos = mangledLookup.find("::", pos)) != std::string::npos) {
            mangledLookup.replace(pos, 2, "__");
            pos += 2;
        }

        std::vector<std::string> candidates;

        if (name.find("::") == std::string::npos) {
            std::vector<std::string> temp = activeDens;
            while (!temp.empty()) {
                std::string prefix = "";
                for (const auto& den : temp) prefix += den + "__";
                candidates.push_back(prefix + name);
                temp.pop_back();
            }
        }

        candidates.push_back(mangledLookup);

        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            for (const auto& cand : candidates) {
                auto found = it->find(cand);
                if (found != it->end()) {
                    return found->second;
                }
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
            ss << den << "__";
        }
        return ss.str();
    }

} // namespace raccoon::compiler::ast