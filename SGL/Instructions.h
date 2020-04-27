#pragma once

#include <cstdint>

#include "SGLTypes.h"

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
	// Pops the top int on the stack, casts to float, and pushes the float
	INT_TO_FLOAT,
	// Pops the top float on the stack, casts to int, and pushes the int
	FLOAT_TO_INT,
	// Invalid instruction, used to denote compilation failures
	INVALID_INSTRUCTION,
	// Number of instructions total
	INSTRUCTION_COUNT
};

inline SGLInstruction get_cast_instruction(SGLType from, SGLType to)
{
	if (from.TypeName == "int32")
	{
		// INT_TO_*
		if (to.TypeName == "float")
		{
			return INT_TO_FLOAT;
		}
		else
		{
			return INVALID_INSTRUCTION;
		}
	}
	else if (from.TypeName == "float")
	{
		// FLOAT_TO_*
		if (to.TypeName == "int32")
		{
			return FLOAT_TO_INT;
		}
		else
		{
			return INVALID_INSTRUCTION;
		}
	}
	else
	{
		return INVALID_INSTRUCTION;
	}
}