#pragma once

#include "Stack.h"

class VirtualMachine
{
public:

	VirtualMachine(size_t stacksize);

	VirtualMachine();

	void execute_bytecode(std::uint8_t* code, size_t bufferSize);

private:

	VMStack _stack;

};