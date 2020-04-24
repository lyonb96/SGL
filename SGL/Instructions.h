#pragma once

#include <cstdint>

enum SGLInstruction : std::uint8_t
{
	// Pushes an integer constant onto the stack
	// Following 4 bytes after this instruction are the int constant
	INT_CONST,
	// Pops the integer on top of the stack and stores it to a variable position
	// Following 1 byte is the variable pos to store to
	INT_STORE,
	// Loads an integer value from a variable position and pushes the value to the stack
	// Following 1 byte is the variable pos to load from
	INT_LOAD,
	// Pops the top two ints on the stack, adds them, and pushes the result
	INT_ADD,
	// Pops the top two ints on the stack, subtracts them (left to right), and pushes the result
	INT_SUB,
	// Pops the top two ints on the stack, multiplies them, and pushes the result
	INT_MUL,
	// Pops the top two ints on the stack, divides them (left to right), and pushes the result
	INT_DIV,
	// Pops the top two ints on the stack, % them (left to right), and pushes the result
	INT_MOD,

	// Number of instructions total
	INSTRUCTION_COUNT
};