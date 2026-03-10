#pragma once

#include <string>
#include <unordered_map>

namespace raccoon::compiler {

    enum class Type {
        UNKNOWN,
        INT,
        FLOAT,
        BOOL,
        VOID,
        FUNCTION
    };


    inline std::string typeToString(Type type) {
        switch (type) {
            case Type::INT:      return "int";
            case Type::FLOAT:    return "float";
            case Type::BOOL:     return "boolean";
            case Type::VOID:     return "void";
            default:             return "unknown";
        }
    }

    inline Type stringToType(const std::string& typeStr) {
        static const std::unordered_map<std::string, Type> typeMap = {
                {"int",    Type::INT},
                {"float",  Type::FLOAT},
                {"boolean",   Type::BOOL},
                {"void",   Type::VOID},
        };

        auto it = typeMap.find(typeStr);
        if (it != typeMap.end()) {
            return it->second;
        }
        return Type::UNKNOWN;
    }
}