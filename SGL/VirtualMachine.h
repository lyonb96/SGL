#pragma once

#include "Stack.h"

class VirtualMachine
{
public:

	VirtualMachine(size_t stacksize);

	VirtualMachine();

	void execute_bytecode(char* code, size_t bufferSize);

private:

	VMStack _stack;

};