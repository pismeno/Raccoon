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

    void IRGenerator::visit(BlockStmt& node) {
        for (const auto& stmt : node.statements) {
            if (stmt) stmt->accept(*this);
        }
    }

    void IRGenerator::visit(VariableDecl& node) {
        llvm::Type* varType = getLLVMType(node.type);

        if (!builder->GetInsertBlock()) {
            std::string mangledName = currentDen.empty() ? node.name : currentDen + "_" + node.name;

            auto* globalVar = new llvm::GlobalVariable(
                    *module, varType, false, llvm::GlobalValue::ExternalLinkage,
                    llvm::Constant::getNullValue(varType), mangledName
            );

            VarInfo info {node.name, node.type, true, true, globalVar};
            varTable.define(node.name, info);

            if (node.initializer) {
                node.initializer->accept(*this);
            }
        } else {
            llvm::AllocaInst* alloca = builder->CreateAlloca(varType, nullptr, node.name);

            VarInfo info {node.name, node.type, true, false, alloca};
            varTable.define(node.name, info);

            if (node.initializer) {
                node.initializer->accept(*this);
                builder->CreateStore(lastValue, alloca);
            }
        }
    }

    void IRGenerator::visit(FunctionDecl &node) {
        std::vector<llvm::Type*> paramTypes;
        for (const auto& pType : node.paramTypes) {
            paramTypes.push_back(getLLVMType(pType));
        }

        llvm::Type* retType = getLLVMType(node.returnType);

        llvm::FunctionType* funcType = llvm::FunctionType::get(retType, paramTypes, false);

        llvm::Function* func = llvm::Function::Create(
                funcType, llvm::Function::ExternalLinkage, node.name, module.get()
        );

        llvm::BasicBlock* backupBlock = builder->GetInsertBlock();

        llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "entry", func);
        builder->SetInsertPoint(entry);

        size_t idx = 0;
        for (auto& arg : func->args()) {
            std::string argName = node.paramNames[idx++];
            arg.setName(argName);

            llvm::AllocaInst* alloca = builder->CreateAlloca(arg.getType(), nullptr, argName);
            builder->CreateStore(&arg, alloca);
        }

        if (node.body) {
            node.body->accept(*this);
        }

        if (!builder->GetInsertBlock()->getTerminator()) {
            if (retType->isVoidTy()) builder->CreateRetVoid();
            else builder->CreateRet(llvm::Constant::getNullValue(retType));
        }

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

    llvm::Type* IRGenerator::getLLVMType(const std::string& declaredTypeString) {
        Type raccoonType = stringToType(declaredTypeString);
        if (raccoonType == Type::VOID)   return builder->getVoidTy();
        if (raccoonType == Type::INT)    return builder->getInt64Ty();
        if (raccoonType == Type::FLOAT) return builder->getDoubleTy();
        if (raccoonType == Type::BOOL)   return builder->getInt1Ty();
        return builder->getInt32Ty();
    }
} // raccoon