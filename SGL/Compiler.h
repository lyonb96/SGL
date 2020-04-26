#pragma once

#include <string>

#include "Script.h"

enum class SGLResult
{
    // Success result
    SGL_OK,
    // General error for when invalid source is passed in but no specific error could be found
    // An example is trying to compile something that isn't SGL, or compiling an empty file
    SGL_ERR_SOURCE_INVALID,
    // Error thrown when an unclosed block comment is detected
    SGL_ERR_UNCLOSED_BLOCK_COMMENT,
    // Error when a line is incomplete (such as a variable type without an identifier)
    SGL_ERR_UNEXPECTED_END_OF_LINE,
    // Error when an unknown type is found
    SGL_ERR_UNKNOWN_TYPE,
    // Error when a missing semicolon is found
    SGL_ERR_MISSING_SEMICOLON,
    // Error when a missing curly brace is found
    SGL_ERR_MISSING_CURLY_BRACE,
    // Error when a missing parentheses is found
    SGL_ERR_MISSING_PARENTHESES,
    // Error when a variable or function identifier is invalid
    SGL_ERR_INVALID_IDENTIFIER,
    // Error when an operator is found but there is no operand to the left of it
    SGL_ERR_MISSING_LEFT_OPERAND,
    // Error when an operator is found but there is no operand to the right of it
    SGL_ERR_MISSING_RIGHT_OPERAND
};

/**
 * Compiles an SGL script represented as a string
 */
SGLResult compile_sgl(std::string source);

/**
 * Runs some test cases against internal compiler functions
 */
void execute_compiler_test();