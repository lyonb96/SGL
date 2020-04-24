#include "Stack.h"

#ifndef SGL_STACK_DEFAULT_SIZE
#define SGL_STACK_DEFAULT_SIZE 1024
#endif

VMStack::VMStack(size_t size)
{
	if (size == 0)
	{
		std::cerr << "Size 0 is invalid size for stack. Using default size: " << SGL_STACK_DEFAULT_SIZE << std::endl
			<< "Note: you can change the default stack size by defining SGL_STACK_DEFAULT_SIZE to a non-zero value." << std::endl;
		size = SGL_STACK_DEFAULT_SIZE;
	}

	_stacksize = size;
	_stackmem = 0;
	_stackpos = 0;
}

bool VMStack::initialize_stack()
{
	if (_stackmem)
	{
		// memory already allocated, bail out
		return true;
	}

	// allocate a buffer for the stack, aligned to int boundary
	_stackmem = static_cast<char*>(_aligned_malloc(_stacksize, 4));

	return (_stackmem != nullptr);
}

void VMStack::shutdown_stack()
{
	if (_stackmem)
	{
		_aligned_free(_stackmem);
		_stackmem = 0;
	}

	_stackpos = 0;
}

VMStack::~VMStack()
{
	shutdown_stack();
}