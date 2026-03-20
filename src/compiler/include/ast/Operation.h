#pragma once

namespace raccoon::compiler::ast {
    enum Operation {
        ADD, SUB, MUL, DIV,
        EQUAL, NOT_EQUAL, GREATER, LESSER, GREATER_EQUAL, LESSER_EQUAL, AND, OR,
    };
} // namespace raccoon::compiler::ast
