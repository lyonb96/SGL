// SGL (Simple Game Language) Compiler

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

#include "SGLTypes.h"
#include "Compiler.h"
#include "VirtualMachine.h"
#include "Instructions.h"
#include "Helpers.h"

int input_loop()
{
	std::string filename;

	// Grab the file to compile
	std::cout << "Insert file name to compile, or enter quit to exit: ";
	std::cin >> filename;

	if (filename.empty())
	{
		// If empty, go back to the top
		return input_loop();
	}
	else if (filename == "quit")
	{
		// If user entered "quit" then quit
		return 0;
	}
	else
	{
		// Make sure the requested file exists
		if (!std::filesystem::exists(filename))
		{
			std::cout << "File " << filename << " does not exist, please try again." << std::endl;
			return input_loop();
		}

		// Open the file
		std::ifstream file;
		file.open(filename);
		if (!file.good())
		{
			// If the file fails to open, start over
			std::cout << "Failed to open file " << filename << ", please try again." << std::endl;
			return input_loop();
		}

		// Read file into string
		std::stringstream sstr;
		sstr << file.rdbuf();
		std::string source = sstr.str();
		file.close();

		// Pass file contents to compiler and check for pass or fail
		SGLResult result = compile_sgl(source);
		if (result != SGLResult::SGL_OK)
		{
			std::cout << "Compilation failed, check for syntax errors and try again." << std::endl;
			return input_loop();
		}
		else
		{
			std::cout << "Compilation successful!" << std::endl;
			return input_loop();
		}
	}
}

int main(const char** argv, int argc)
{
	// Hello world in SGL
	std::string test = "func: Hello() { print(\"Hello, world!\"); }";

	register_datatypes();

	execute_compiler_test();

	{
		VirtualMachine vm;
		std::uint8_t bytecode[98];
		bytecode[0] = INT_CONST; store_to_buffer<int>(&bytecode[1], 97, 5);		// INT_CONST 5
		bytecode[5] = INT_STORE; bytecode[6] = 0;								// INT_STORE 0 (int32 x = 5)
		bytecode[7] = INT_CONST; store_to_buffer<int>(&bytecode[8], 90, 12);	// INT_CONST 12
		bytecode[12] = INT_STORE; bytecode[13] = 1;								// INT_STORE 1 (int32 y = 12)
		bytecode[14] = INT_CONST; store_to_buffer<int>(&bytecode[15], 83, 6);	// INT_CONST 6
		bytecode[19] = INT_STORE; bytecode[20] = 2;								// INT_STORE 2 (int32 z = 6)
		bytecode[21] = INT_CONST; store_to_buffer<int>(&bytecode[22], 76, 8);	// INT_CONST 8
		bytecode[26] = INT_STORE; bytecode[27] = 3;								// INT_STORE 3 (int32 w = 8)
		bytecode[28] = INT_CONST; store_to_buffer<int>(&bytecode[29], 69, 10);	// INT_CONST 10
		bytecode[33] = INT_LOAD; bytecode[34] = 3;								// INT_LOAD 3
		bytecode[35] = INT_LOAD; bytecode[36] = 2;								// INT_LOAD 2
		bytecode[37] = INT_CONST; store_to_buffer<int>(&bytecode[38], 60, 8);	// INT_CONST 8
		bytecode[42] = INT_LOAD; bytecode[43] = 0;								// INT_LOAD 0
		bytecode[44] = INT_MUL;													// INT_MUL (8 * x)
		bytecode[45] = INT_MUL;													// INT_MUL (z * (8 * x))
		bytecode[46] = INT_ADD;													// INT_ADD (w + z * (8 * x))
		bytecode[47] = INT_MUL;													// INT_MUL (10 * (w + z * (8 * x)))
		bytecode[48] = INT_LOAD; bytecode[49] = 1;								// INT_LOAD 1
		bytecode[50] = INT_MOD;													// INT_MOD (10 * (w + z * (8 * x)) % y
		bytecode[51] = INT_LOAD; bytecode[52] = 0;								// INT_LOAD 0
		bytecode[53] = INT_CONST; store_to_buffer<int>(&bytecode[54], 44, 1);	// INT_CONST 1
		bytecode[58] = INT_ADD;													// INT_ADD (x + 1)
		bytecode[59] = INT_DIV;													// INT_DIV (10 * (w + z * (8 * x)) % y / (x + 1))
		bytecode[60] = INT_STORE; bytecode[61] = 4;								// INT_STORE 4 (i = result of above)

		vm.execute_bytecode(bytecode, 61);
	}

	int x = 5;
	int y = 12;
	int z = 6;
	int w = 8;
	int i = 10 * (w + z * (8 * x)) % y / (x + 1);
	//  i = 10 * (w + z * (  40 )) % y / (x + 1)
	//  i = 10 * (w + (    240   ) % y / (x + 1)
	//  i = 10 * (      248      ) % y / (  6  )
	//  i = 2480 % 12 / 6
	//  i = 8 / 6
	//  i = 1

	std::cout << "Result in C++: " << i << std::endl;

	return input_loop();
}