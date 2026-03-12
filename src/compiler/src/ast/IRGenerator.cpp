#include "../../include/ast/AST.hpp"
#include "../../include/ast/IRGenerator.hpp"
#include "../../include/Parser.hpp"

namespace raccoon::compiler::ast {

    IRGenerator::IRGenerator() {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("RaccoonModule", *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
    }

    void IRGenerator::visit(LiteralExpr& node) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) this->lastValue = llvm::ConstantInt::get(builder->getInt64Ty(), arg);
            else if constexpr (std::is_same_v<T, double>) this->lastValue = llvm::ConstantFP::get(builder->getDoubleTy(), arg);
            else if constexpr (std::is_same_v<T, bool>)   this->lastValue = builder->getInt1(arg);
        }, node.value);
    }

    void IRGenerator::visit(VariableExpr& node) {
        std::optional<VarInfo> varInfo = varTable.lookup(node.name);
        if (!varInfo) throw ParseError("Undefined variable: " + node.name);

        if (builder->GetInsertBlock()) {
            llvm::Type* loadType = getLLVMType(varInfo->type);
            this->lastValue = builder->CreateLoad(loadType, varInfo->address, node.name.c_str());
        } else {
            this->lastValue = varInfo->address;
        }
    }

    void IRGenerator::visit(UnaryExpression& node) {
        if (node.expr) node.expr->accept(*this);
        llvm::Value* operandValue = this->lastValue;

        if (!operandValue) {
            throw ParseError("Failed to generate IR for unary operand.");
        }

        bool isFloat = operandValue->getType()->isFloatingPointTy();

        if (node.op == "-") {
            this->lastValue = isFloat ? builder->CreateFNeg(operandValue, "negtmp")
                                      : builder->CreateNeg(operandValue, "negtmp");
        } else if (node.op == "!") {
            this->lastValue = builder->CreateNot(operandValue, "nottmp");
        } else {
            throw ParseError("Unknown unary operator: " + node.op);
        }
    }

    void IRGenerator::visit(BinaryExpression& node) {
        if (node.left) node.left->accept(*this);
        llvm::Value* leftValue = this->lastValue;

        if (node.right) node.right->accept(*this);
        llvm::Value* rightValue = this->lastValue;

        if (!leftValue || !rightValue) {
            throw ParseError("Failed to generate IR for binary operands.");
        }

        bool isFloat = leftValue->getType()->isFloatingPointTy();

        if (node.op == "+") {
            this->lastValue = isFloat ? builder->CreateFAdd(leftValue, rightValue, "addtmp")
                                      : builder->CreateAdd(leftValue, rightValue, "addtmp");
        } else if (node.op == "-") {
            this->lastValue = isFloat ? builder->CreateFSub(leftValue, rightValue, "subtmp")
                                      : builder->CreateSub(leftValue, rightValue, "subtmp");
        } else if (node.op == "*") {
            this->lastValue = isFloat ? builder->CreateFMul(leftValue, rightValue, "multmp")
                                      : builder->CreateMul(leftValue, rightValue, "multmp");
        } else if (node.op == "/") {
            this->lastValue = isFloat ? builder->CreateFDiv(leftValue, rightValue, "divtmp")
                                      : builder->CreateSDiv(leftValue, rightValue, "divtmp");
        } else {
            throw ParseError("Unknown binary operator: " + node.op);
        }
    }

    void IRGenerator::visit(BlockStmt& node) {
        for (const auto& stmt : node.statements) {
            if (stmt) stmt->accept(*this);
        }
    }

    void IRGenerator::visit(VariableDecl& node) {
        std::shared_ptr<Type> nodeType = stringToType(node.type);
        llvm::Type* varType = getLLVMType(nodeType);

        if (!builder->GetInsertBlock()) {
            std::string mangledName = currentDen.empty() ? node.name : currentDen + "_" + node.name;

            llvm::Constant* initConst = llvm::Constant::getNullValue(varType);
            if (node.initializer) {
                node.initializer->accept(*this);
                if (auto* c = llvm::dyn_cast<llvm::Constant>(lastValue)) {
                    initConst = c;
                } else {
                    throw ParseError("Global initializer must be a constant.");
                }
            }

            auto* globalVar = new llvm::GlobalVariable(
                    *module, varType, false, llvm::GlobalValue::ExternalLinkage,
                    initConst, mangledName
            );

            VarInfo info {node.name, nodeType, true, true, globalVar};
            varTable.define(node.name, info);
        } else {
            llvm::AllocaInst* alloca = builder->CreateAlloca(varType, nullptr, node.name);

            VarInfo info {node.name, nodeType, true, false, alloca};
            varTable.define(node.name, info);

            if (node.initializer) {
                node.initializer->accept(*this);
                builder->CreateStore(lastValue, alloca);
            }
        }
    }

    void IRGenerator::visit(FunctionDecl &node) {
        std::vector<llvm::Type*> paramTypes;
        for (const auto& pTypeStr : node.paramTypes) {
            paramTypes.push_back(getLLVMType(stringToType(pTypeStr)));
        }

        llvm::Type* retType = getLLVMType(stringToType(node.returnType));
        llvm::FunctionType* funcType = llvm::FunctionType::get(retType, paramTypes, false);

        llvm::Function* func = llvm::Function::Create(
                funcType, llvm::Function::ExternalLinkage, node.name, module.get()
        );

        llvm::BasicBlock* backupBlock = builder->GetInsertBlock();
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "entry", func);
        builder->SetInsertPoint(entry);

        varTable.enterScope();

        size_t idx = 0;
        for (auto& arg : func->args()) {
            std::string argName = node.paramNames[idx];
            arg.setName(argName);

            llvm::AllocaInst* alloca = builder->CreateAlloca(arg.getType(), nullptr, argName);
            builder->CreateStore(&arg, alloca);

            std::shared_ptr<Type> pType = stringToType(node.paramTypes[idx]);
            VarInfo info {argName, pType, true, false, alloca};
            varTable.define(argName, info);

            idx++;
        }

        if (node.body) {
            node.body->accept(*this);
        }

        if (!builder->GetInsertBlock()->getTerminator()) {
            if (retType->isVoidTy()) builder->CreateRetVoid();
            else builder->CreateRet(llvm::Constant::getNullValue(retType));
        }

        varTable.exitScope();

        if (backupBlock) {
            builder->SetInsertPoint(backupBlock);
        } else {
            builder->ClearInsertionPoint();
        }

        this->lastValue = func;
    }

    void IRGenerator::visit(DenStmt& node) {
        std::string oldDen = currentDen;
        currentDen = node.name;
        varTable.enterScope(node.name);

        for (const auto& stmt : node.contents) {
            if (stmt) stmt->accept(*this);
        }

        varTable.exitScope();
        currentDen = oldDen;
    }

    llvm::Type* IRGenerator::getLLVMType(std::shared_ptr<Type> type) {
        if (!type) return builder->getVoidTy();

        switch (type->getKind()) {
            case TypeKind::VOID:   return builder->getVoidTy();
            case TypeKind::INT:    return builder->getInt64Ty();
            case TypeKind::FLOAT:  return builder->getDoubleTy();
            case TypeKind::BOOL:   return builder->getInt1Ty();
            default:               return builder->getInt32Ty();
        }
    }
} // namespace raccoon