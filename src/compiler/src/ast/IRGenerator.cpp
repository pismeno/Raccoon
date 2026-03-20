#include "../../include/ast/AST.hpp"
#include "../../include/ast/IRGenerator.hpp"
#include "../../include/Parser.hpp"

namespace raccoon::compiler::ast {

    IRGenerator::IRGenerator() {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("RaccoonModule", *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);


    }

    void IRGenerator::visit(ExprStmt &node) {
        if (node.expr) node.expr->accept(*this);
    }

    void IRGenerator::visit(LiteralExpr& node) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int32_t>) this->lastValue = llvm::ConstantInt::get(builder->getInt32Ty(), arg);
            else if constexpr (std::is_same_v<T, double>) this->lastValue = llvm::ConstantFP::get(builder->getDoubleTy(), arg);
            else if constexpr (std::is_same_v<T, bool>)   this->lastValue = builder->getInt1(arg);
        }, node.value);
    }

    void IRGenerator::visit(VariableExpr& node) {
        std::optional<VarInfo> varInfo = varTable.lookup(node.name);
        if (!varInfo) throw ParseError("Undefined variable: " + node.name);

        if (builder->GetInsertBlock()) {
            if (!varInfo->isMutable && varInfo->type->getKind() == TypeKind::FUNCTION) {
                this->lastValue = varInfo->address;
            } else {
                llvm::Type* loadType = getLLVMType(varInfo->type);
                this->lastValue = builder->CreateLoad(loadType, varInfo->address, node.name.c_str());
            }
        } else {
            this->lastValue = varInfo->address;
        }
    }

    void IRGenerator::visit(FunctionExpr& node) {
        std::shared_ptr<FunctionType> signature = this->currentExpectedFunctionType;
        if (!signature) throw ParseError("Missing function signature during IR generation.");

        std::string funcName = this->currentExpectedFuncName;
        if (funcName.empty()) {
            static int lambdaCount = 0;
            funcName = "__lambda_" + std::to_string(lambdaCount++);
        }

        std::vector<llvm::Type*> paramTypes;
        for (const auto& pt : signature->params) {
            paramTypes.push_back(getLLVMType(pt));
        }
        llvm::Type* retType = getLLVMType(signature->returnType);
        llvm::FunctionType* funcType = llvm::FunctionType::get(retType, paramTypes, false);

        llvm::Function* func = llvm::Function::Create(
                funcType, llvm::Function::ExternalLinkage, funcName, module.get()
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

            VarInfo info {argName, signature->params[idx], false, false, alloca};
            varTable.define(argName, info);
            idx++;
        }

        if (node.body) {
            node.body->accept(*this);
        }

        varTable.exitScope();

        if (backupBlock) {
            builder->SetInsertPoint(backupBlock);
        } else {
            builder->ClearInsertionPoint();
        }

        this->lastValue = func;
    }

    void IRGenerator::visit(CallExpr &node) {

        if (node.func == "print") {
            llvm::FunctionCallee printfFn = module->getOrInsertFunction(
                    "printf",
                    llvm::FunctionType::get(builder->getInt32Ty(), {builder->getPtrTy()}, true)
            );

            if (node.args.size() != 1) throw ParseError("print() expects 1 argument.");
            node.args[0]->accept(*this);
            llvm::Value* valToPrint = this->lastValue;

            std::string fmt;
            if (valToPrint->getType()->isIntegerTy(32)) fmt = "%d\n";
            else throw ParseError("Unsupported type for print()");

            llvm::Value* fmtPtr = builder->CreateGlobalStringPtr(fmt, "print_fmt");

            this->lastValue = builder->CreateCall(printfFn, {fmtPtr, valToPrint});
        } else {
            llvm::Function* function = module->getFunction(node.func);

            if (!function) throw ParseError("Undefined function: " + node.func);

            std::vector<llvm::Value *> args;
            for (const auto& arg : node.args) {
                arg->accept(*this);
                args.push_back(this->lastValue);
            }

            llvm::CallInst* call = builder->CreateCall(function, args);

            this->lastValue = call;
        }
    }

    void IRGenerator::visit(ReturnStmt &node) {
        if (node.expr) {
            node.expr->accept(*this);
        }

        llvm::BasicBlock* currentBlock = builder->GetInsertBlock();
        if (!currentBlock) throw ParseError("Return statement outside of a valid block.");

        llvm::Function* currentFunc = currentBlock->getParent();
        llvm::Type* retType = currentFunc->getReturnType();

        if (retType->isVoidTy()) {
            builder->CreateRetVoid();
        } else {
            builder->CreateRet(this->lastValue ? this->lastValue : llvm::Constant::getNullValue(retType));
        }
    }

    void IRGenerator::visit(UnaryExpression& node) {
        if (node.expr) node.expr->accept(*this);
        llvm::Value* operandValue = this->lastValue;

        if (!operandValue) throw ParseError("Failed to generate IR for unary operand.");

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

        if (!leftValue || !rightValue) throw ParseError("Failed to generate IR for binary operands.");

        bool isFloat = leftValue->getType()->isFloatingPointTy();

        switch (node.op) {
            case Operation::ADD:
                this->lastValue = isFloat ? builder->CreateFAdd(leftValue, rightValue, "addtmp") : builder->CreateAdd(leftValue, rightValue, "addtmp");
                break;
            case Operation::SUB:
                this->lastValue = isFloat ? builder->CreateFSub(leftValue, rightValue, "subtmp") : builder->CreateSub(leftValue, rightValue, "subtmp");
                break;
            case Operation::MUL:
                this->lastValue = isFloat ? builder->CreateFMul(leftValue, rightValue, "multmp") : builder->CreateMul(leftValue, rightValue, "multmp");
                break;
            case Operation::DIV:
                this->lastValue = isFloat ? builder->CreateFDiv(leftValue, rightValue, "divtmp") : builder->CreateSDiv(leftValue, rightValue, "divtmp");
                break;
            case Operation::LESSER:
                this->lastValue = isFloat ? builder->CreateFCmpOLT(leftValue, rightValue, "cmptmp") : builder->CreateICmpSLT(leftValue, rightValue, "cmptmp");
                break;
            case Operation::GREATER:
                this->lastValue = isFloat ? builder->CreateFCmpOGT(leftValue, rightValue, "cmptmp") : builder->CreateICmpSGT(leftValue, rightValue, "cmptmp");
                break;
            case Operation::LESSER_EQUAL:
                this->lastValue = isFloat ? builder->CreateFCmpOLE(leftValue, rightValue, "cmptmp") : builder->CreateICmpSLE(leftValue, rightValue, "cmptmp");
                break;
            case Operation::GREATER_EQUAL:
                this->lastValue = isFloat ? builder->CreateFCmpOGE(leftValue, rightValue, "cmptmp") : builder->CreateICmpSGE(leftValue, rightValue, "cmptmp");
                break;
            case Operation::EQUAL:
                this->lastValue = isFloat ? builder->CreateFCmpOEQ(leftValue, rightValue, "cmptmp") : builder->CreateICmpEQ(leftValue, rightValue, "cmptmp");
                break;
            case Operation::NOT_EQUAL:
                this->lastValue = isFloat ? builder->CreateFCmpONE(leftValue, rightValue, "cmptmp") : builder->CreateICmpNE(leftValue, rightValue, "cmptmp");
                break;
            case Operation::AND:
                this->lastValue = builder->CreateAnd(leftValue, rightValue, "andtmp");
                break;
            case Operation::OR:
                this->lastValue = builder->CreateOr(leftValue, rightValue, "ortmp");
                break;

            default:
                throw ParseError("Unknown binary operator during IR generation");
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

        llvm::Value* initValue = llvm::Constant::getNullValue(varType);

        if (node.initializer) {
            auto prevExpectedType = this->currentExpectedFunctionType;
            auto prevExpectedName = this->currentExpectedFuncName;

            if (nodeType->getKind() == TypeKind::FUNCTION) {
                this->currentExpectedFunctionType = std::dynamic_pointer_cast<FunctionType>(nodeType);
                this->currentExpectedFuncName = node.name;
            }

            node.initializer->accept(*this);
            initValue = this->lastValue;

            this->currentExpectedFunctionType = prevExpectedType;
            this->currentExpectedFuncName = prevExpectedName;
        }

        llvm::BasicBlock* backupBlock = builder->GetInsertBlock();
        bool isGlobal = (backupBlock == nullptr);

        if (!node.isMutable && nodeType->getKind() == TypeKind::FUNCTION) {
            VarInfo info {node.name, nodeType, false, isGlobal, initValue};
            varTable.define(node.name, info);
        } else if (isGlobal) {
            std::string mangledName = currentDen.empty() ? node.name : currentDen + "_" + node.name;
            auto* constInit = llvm::dyn_cast<llvm::Constant>(initValue);
            if (!constInit) throw ParseError("Global initializer must be a constant.");

            auto* globalVar = new llvm::GlobalVariable(
                    *module, varType, !node.isMutable, llvm::GlobalValue::ExternalLinkage,
                    constInit, mangledName
            );

            VarInfo info {node.name, nodeType, node.isMutable, true, globalVar};
            varTable.define(node.name, info);
        } else {
            llvm::AllocaInst* alloca = builder->CreateAlloca(varType, nullptr, node.name);
            builder->CreateStore(initValue, alloca);

            VarInfo info {node.name, nodeType, node.isMutable, false, alloca};
            varTable.define(node.name, info);
        }
    }

    void IRGenerator::visit(VariableAssign& node) {
        std::optional<VarInfo> varInfo = varTable.lookup(node.name);
        if (!varInfo) throw ParseError("Undefined variable: " + node.name);

        if (node.value) {
            if (varInfo->type->getKind() == TypeKind::FUNCTION) {
                this->currentExpectedFunctionType = std::dynamic_pointer_cast<FunctionType>(varInfo->type);
                this->currentExpectedFuncName = "";
            }

            node.value->accept(*this);
            this->currentExpectedFunctionType = nullptr;
            this->currentExpectedFuncName = "";
        }

        builder->CreateStore(this->lastValue, varInfo->address);
    }

    void IRGenerator::visit(FunctionDecl &node) {
        std::vector<std::shared_ptr<Type>> raccoonParamTypes;
        for (const auto& paramType : node.paramTypes) {
            raccoonParamTypes.push_back(stringToType(paramType));
        }
        std::shared_ptr<FunctionType> signature = FunctionType::make(stringToType(node.returnType), raccoonParamTypes);

        llvm::Value* resolvedFuncPtr = nullptr;

        if (node.initializer) {
            this->currentExpectedFunctionType = signature;
            this->currentExpectedFuncName = node.name;

            node.initializer->accept(*this);
            resolvedFuncPtr = this->lastValue;

            this->currentExpectedFunctionType = nullptr;
            this->currentExpectedFuncName = "";
        } else {
            std::vector<llvm::Type*> paramTypes;
            for (const auto& pTypeStr : node.paramTypes) {
                paramTypes.push_back(getLLVMType(stringToType(pTypeStr)));
            }
            llvm::FunctionType* funcType = llvm::FunctionType::get(getLLVMType(stringToType(node.returnType)), paramTypes, false);
            resolvedFuncPtr = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.name, module.get());
        }

        llvm::BasicBlock* backupBlock = builder->GetInsertBlock();
        bool isGlobal = (backupBlock == nullptr);

        if (!node.isMutable) {
            VarInfo varInfo {node.name, signature, false, isGlobal, resolvedFuncPtr};
            varTable.define(node.name, varInfo);
        } else {
            llvm::Value* memory_box = nullptr;
            if (isGlobal) {
                auto* constInit = llvm::dyn_cast<llvm::Constant>(resolvedFuncPtr);
                memory_box = new llvm::GlobalVariable(
                        *module, builder->getPtrTy(), false, llvm::GlobalValue::ExternalLinkage,
                        constInit, node.name + "_mut_var"
                );
            } else {
                memory_box = builder->CreateAlloca(builder->getPtrTy(), nullptr, node.name + "_ptr");
                builder->CreateStore(resolvedFuncPtr, memory_box);
            }
            VarInfo varInfo {node.name, signature, true, isGlobal, memory_box};
            varTable.define(node.name, varInfo);
        }

        this->lastValue = resolvedFuncPtr;
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
            case TypeKind::VOID:     return builder->getVoidTy();
            case TypeKind::INT:      return builder->getInt32Ty();
            case TypeKind::FLOAT:    return builder->getDoubleTy();
            case TypeKind::BOOL:     return builder->getInt1Ty();
            case TypeKind::FUNCTION: return builder->getPtrTy();
            default:                 return builder->getInt32Ty();
        }
    }
} // namespace raccoon::compiler::ast