#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace raccoon::compiler::ast {

    enum TypeKind {
        UNKNOWN, INT, FLOAT, BOOL, VOID, FUNCTION, CLASS, OBJECT
    };

    class Type {
    public:
        Type(TypeKind kind) : kind(kind) {}
        virtual ~Type() = default;

        bool operator==(const Type& other) const {
            if (this->kind != other.kind) return false;
            return this->equals(other);
        }

        bool operator!=(const Type& other) const { return !(*this == other); }

        [[nodiscard]] TypeKind getKind() const { return kind; }
    protected:
        TypeKind kind;
        [[nodiscard]] virtual bool equals(const Type& other) const = 0;
    };

    class PrimitiveType : public Type {
    public:
        PrimitiveType(TypeKind kind) : Type(kind) {}

        static const std::shared_ptr<PrimitiveType> Unknown;
        static const std::shared_ptr<PrimitiveType> Int;
        static const std::shared_ptr<PrimitiveType> Float;
        static const std::shared_ptr<PrimitiveType> Bool;
        static const std::shared_ptr<PrimitiveType> Void;
        static const std::shared_ptr<PrimitiveType> Class;

    protected:
        [[nodiscard]] bool equals(const Type& other) const override { return true; }
    };

    class FunctionType : public Type {
    public:
        std::shared_ptr<Type> returnType;
        std::vector<std::shared_ptr<Type>> params;

        FunctionType(std::shared_ptr<Type> returnType,
                     std::vector<std::shared_ptr<Type>> paramTypes) :
                Type(TypeKind::FUNCTION),
                returnType(std::move(returnType)),
                params(std::move(paramTypes)) {}

        static std::shared_ptr<FunctionType> make(const std::shared_ptr<Type>& ret,
                                                  const std::vector<std::shared_ptr<Type>>& params) {
            return std::make_shared<FunctionType>(ret, params);
        }

    protected:
        [[nodiscard]] bool equals(const Type& other) const override;
    };

    class ObjectType : public Type {
    public:
        std::string classVariableName;

        ObjectType(std::string varName)
                : Type(TypeKind::OBJECT), classVariableName(std::move(varName)) {}

    protected:
        [[nodiscard]] bool equals(const Type& other) const override {
            if (other.getKind() != TypeKind::OBJECT) return false;
            auto otherObj = dynamic_cast<const ObjectType*>(&other);
            return classVariableName == otherObj->classVariableName;
        }
    };

    inline std::string typeToString(Type* type) {
        switch (type->getKind()) {
            case TypeKind::INT: return "int";
            case TypeKind::FLOAT: return "float";
            case TypeKind::BOOL: return "boolean";
            case TypeKind::VOID: return "void";
            case TypeKind::FUNCTION: return "function";
            case TypeKind::CLASS: return "class";
            default: return "unknown";
        }
    }



    inline TypeKind stringToTypeKind(const std::string& typeStr) {
        static const std::unordered_map<std::string, TypeKind> typeMap = {
                {"int", TypeKind::INT},
                {"float", TypeKind::FLOAT},
                {"boolean", TypeKind::BOOL},
                {"void", TypeKind::VOID},
                {"function", TypeKind::FUNCTION},
                {"class", TypeKind::CLASS},
        };

        auto it = typeMap.find(typeStr);
        if (it != typeMap.end()) {
            return it->second;
        }
        return TypeKind::UNKNOWN;
    }



    inline std::shared_ptr<Type> stringToType(const std::string& typeStr) {
        static const std::unordered_map<std::string, std::shared_ptr<Type>> typeMap = {
                {"int", PrimitiveType::Int},
                {"float", PrimitiveType::Float},
                {"boolean", PrimitiveType::Bool},
                {"void", PrimitiveType::Void},
                {"class", PrimitiveType::Class},
        };

        auto it = typeMap.find(typeStr);
        if (it != typeMap.end()) {
            return it->second;
        }

        return nullptr;
    }
} // namespace raccoon::compiler::ast