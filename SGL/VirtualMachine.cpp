#include "VirtualMachine.h"

#include "Helpers.h"
#include "Instructions.h"

#include <iostream>

VirtualMachine::VirtualMachine(size_t stacksize)
	: _stack(stacksize)
{
	_stack.initialize_stack();
}

VirtualMachine::VirtualMachine()
	: VirtualMachine(0)
{}

void VirtualMachine::execute_bytecode(std::uint8_t* code, size_t bufferSize)
{
	if (code)
	{
		bool isDone = false;
		size_t execPos = 0;
		while (!isDone && execPos < bufferSize)
		{
			std::uint8_t instruction = (code[execPos++]);
			switch (instruction)
			{
				case INT_CONST:
				{
					// next 4 bytes are the constant to push
					int constant = read_from_buffer<int>(code + execPos);
					_stack.push<int>(constant);
					execPos += sizeof(int);
					break;
				}
				case INT_STORE:
				{
					// piss. now i have to write the variable store.
					break;
				}
				case INT_LOAD:
				{
					break;
				}
				case INT_ADD:
				{
					// pop the top two
					int top = _stack.pop<int>();
					int bottom = _stack.pop<int>();
					// add them
					int result = bottom + top;
					// push result
					_stack.push<int>(result);
					break;
				}
				case INT_SUB:
				{
					// pop the top two
					int top = _stack.pop<int>();
					int bottom = _stack.pop<int>();
					// subtract them
					int result = bottom - top;
					// push result
					_stack.push<int>(result);
					break;
				}
				case INT_MUL:
				{
					// pop the top two
					int top = _stack.pop<int>();
					int bottom = _stack.pop<int>();
					// multiply them
					int result = bottom * top;
					// push result
					_stack.push<int>(result);
					break;
				}
				case INT_DIV:
				{
					// pop the top two
					int top = _stack.pop<int>();
					int bottom = _stack.pop<int>();
					// divide them
					int result = bottom / top;
					// push result
					_stack.push<int>(result);
					break;
				}
				case INT_MOD:
				{
					// pop the top two
					int top = _stack.pop<int>();
					int bottom = _stack.pop<int>();
					// modulo them
					int result = bottom % top;
					// push result
					_stack.push<int>(result);
					break;
				}
				default:
				{
					std::cerr << "Unknown instruction detected, byte code " << instruction << ". Terminating." << std::endl;
					isDone = true;
					break;
				}
			}
		}
	}
}