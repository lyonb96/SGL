#pragma once

#include <cstdint>
#include <iostream>

class VMStack
{
public:

	VMStack(size_t size);

	/**
	 * Initializes the stack
	 */
	bool initialize_stack();

	/**
	 * Frees memory and cleans up
	 */
	void shutdown_stack();

	/**
	 * Pops the top of the stack and returns the item as the requested type
	 */
	template <class T>
	T pop()
	{
		size_t Tsize = sizeof(T);
#ifdef _DEBUG
		// in debug builds, verify that this is a valid pop
		if (_stackpos < Tsize)
		{
			std::cerr << "INVALID STACK POP, REQUESTED SIZE " << Tsize << " BYTES EXCEEDS STORED STACK VALUES" << std::endl;
			// die();
		}
#endif
		size_t pos = (_stackpos -= Tsize);
		union
		{
			char* as_char;
			T* as_T;
		};
		as_char = (_stackmem + pos);

		return *as_T;
	}

	/**
	 * Pushes the requested value to the top of the stack
	 */
	template <class T>
	void push(const T& value)
	{
		size_t Tsize = sizeof(T);
#ifdef _DEBUG
		// make sure we're not exceeding the stack size
		if (_stackpos + Tsize > _stacksize)
		{
			std::cerr << "STACK OVERFLOW DETECTED" << std::endl;
			// die();
		}
#endif

		union
		{
			char* as_char;
			T* as_T;
		};

		as_char = (_stackmem + _stackpos);
		*as_T = value;

		_stackpos += Tsize;
	}

	/**
	 * Just in case shutdown_stack() doesn't get called, this cleans up too
	 */
	~VMStack();

private:

	// Pointer to memory allocated for the stack
	char* _stackmem;
	// Size of the memory allocated for the stack
	size_t _stacksize;
	// Read/write position in the stack
	size_t _stackpos;

};