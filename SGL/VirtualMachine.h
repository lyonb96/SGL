#pragma once

#include <vector>

#include "Stack.h"

class VirtualMachine
{
public:

	VirtualMachine(size_t stacksize);

	VirtualMachine();

	void execute_bytecode(std::uint8_t* code, size_t bufferSize);

	~VirtualMachine();

private:

	// working stack
	VMStack _stack;
	// variable map
	std::vector<void*> _variables;

};