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

	execute_compiler_test();

	// register some types
	register_type<int32_t>("int32");
	register_type<uint32_t>("uint32");
	register_type<float>("float");
	register_type<std::string>("string");

	VirtualMachine vm;
	std::uint8_t bytecode[11];
	bytecode[0] = INT_CONST;
	bytecode[1] = 0;
	bytecode[2] = 0;
	bytecode[3] = 1;
	bytecode[4] = 0;
	bytecode[5] = INT_CONST;
	bytecode[6] = 0;
	bytecode[7] = 1;
	bytecode[8] = 0;
	bytecode[9] = 0;
	bytecode[10] = INT_ADD;
	vm.execute_bytecode(bytecode, 11);

	return input_loop();
}