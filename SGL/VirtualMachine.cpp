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
					// grab the byte that corresponds to the slot to store
					std::uint8_t byte = code[execPos++];

					// grow as necessary
					while (byte >= _variables.size())
					{
						_variables.push_back(nullptr);
					}

					if (_variables[byte] != nullptr)
					{
						// see if int is already stored (updating the value)
						int* var = static_cast<int*>(_variables[byte]);
						*var = _stack.pop<int>();
					}
					else
					{
						// allocate new int and store it
						int* var = new int(_stack.pop<int>());
						_variables[byte] = var;
					}
					
					break;
				}
				case INT_LOAD:
				{
					// grab slot to load from
					std::uint8_t byte = code[execPos++];

					// grab pointer to int
					int* ptr = static_cast<int*>(_variables[byte]);
					if (ptr)
					{
						// load int
						_stack.push<int>(*ptr);
					}

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
					// % them
					int result = bottom % top;
					// push result
					_stack.push<int>(result);
					break;
				}
				case INT_TO_FLOAT:
				{
					// pop the int
					int from = _stack.pop<int>();
					// cast to float
					float to = (float)from;
					// push the float
					_stack.push<float>(to);
					break;
				}
				case FLOAT_TO_INT:
				{
					// pop the float
					float from = _stack.pop<float>();
					// cast to int
					int to = (int)from;
					// push the int
					_stack.push<int>(to);
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

VirtualMachine::~VirtualMachine()
{
	// free any allocated variables that didn't get freed
	for (auto ptr : _variables)
	{
		if (ptr)
		{
			delete ptr;
		}
	}
}