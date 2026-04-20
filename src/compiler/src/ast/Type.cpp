#include "../../include/ast/Type.hpp"

namespace raccoon::compiler::ast {

    const std::shared_ptr<PrimitiveType> PrimitiveType::Unknown = std::make_shared<PrimitiveType>(UNKNOWN);
    const std::shared_ptr<PrimitiveType> PrimitiveType::Int = std::make_shared<PrimitiveType>(INT);
    const std::shared_ptr<PrimitiveType> PrimitiveType::Float = std::make_shared<PrimitiveType>(FLOAT);
    const std::shared_ptr<PrimitiveType> PrimitiveType::Bool = std::make_shared<PrimitiveType>(BOOL);
    const std::shared_ptr<PrimitiveType> PrimitiveType::Void = std::make_shared<PrimitiveType>(VOID);
    const std::shared_ptr<PrimitiveType> PrimitiveType::Class = std::make_shared<PrimitiveType>(CLASS);

    bool FunctionType::equals(const Type& other) const {
        const auto& otherFn = static_cast<const FunctionType&>(other);

        if (!(*returnType == *otherFn.returnType)) return false;
        if (params.size() != otherFn.params.size()) return false;

        for (size_t i = 0; i < params.size(); ++i) {
            if (!(*params[i] == *otherFn.params[i])) return false;
        }
        return true;
    }
} // namespace raccoon::compiler::ast