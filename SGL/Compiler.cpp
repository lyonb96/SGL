#include "Compiler.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "StringHelpers.h"

namespace SGL
{
	/**
	 *****************************************************************
	 *			Intermediate helper types for compilation
	 *****************************************************************
	 */

	/**
	 * Struct that holds information about types in SGL
	 */
	struct TypeData
	{
		// Type's name in SGL code
		std::string TypeName;

		/**
		 * Returns true if the type is valid
		 */
		bool is_valid() const
		{
			return !TypeName.empty();
		}

		/**
		 * Comparison operators
		 */
		friend bool operator==(const TypeData& lh, const TypeData& rh)
		{
			return lh.TypeName == rh.TypeName;
		}

		friend bool operator!=(const TypeData& lh, const TypeData& rh)
		{
			return lh.TypeName != rh.TypeName;
		}
	};

	/**
	 * POD types
	 */
	namespace Types
	{
		// 32-bit signed in
		TypeData int_type = { "int32" };
		// 32-bit floating point
		TypeData float_type = { "float" };
		// no-data type
		TypeData void_type = { "void" };
		// invalid type used in the compiler for errors
		TypeData invalid_type{ "" };
	}

	// List of registered types
	std::vector<TypeData> g_types = { Types::int_type, Types::float_type, Types::void_type };

	/**
	 * Returns the TypeData for a given type name
	 * Returns Types::invalid_type for an unrecognized type
	 */
	TypeData get_type(const std::string& typeStr)
	{
		for (auto type : g_types)
		{
			if (type.TypeName == typeStr)
			{
				return type;
			}
		}

		return Types::invalid_type;
	}

	/**
	 * Struct that holds information about functions delcared in the SGL script
	 */
	struct FunctionData
	{
		// Original source code of the function
		std::string FunctionSource;
		// Name to call function
		std::string FunctionName;
		// Return type of the function
		TypeData ReturnType;
		
		/**
		 * Struct to hold function parameter info
		 */
		struct FunctionParam
		{
			TypeData ParamType;
			std::string ParamName;
		};

		// Array of parameters for the function
		std::vector<FunctionParam> FunctionParams;

		bool is_valid() const
		{
			return FunctionName.length() > 0;
		}
	};

	/**
	 * Holds intermediate compiler data
	 */
	struct CompilerState
	{
		std::vector<FunctionData> Functions;
	};

	/**
	 * Commonly used predicates
	 */

	// Returns true for whitespace characters
	auto g_is_whitespace = [](unsigned char c) -> bool { return is_whitespace(c); };

	// Returns true for newline characters
	auto g_is_newline = [](unsigned char c) -> bool { return is_newline(c); };

	// Returns true for newline or whitespace characters
	auto g_is_newline_or_whitespace = [](unsigned char c) -> bool { return is_newline(c) || is_whitespace(c); };

	// Returns true for alphanumeric characters
	auto g_is_alnum = [](unsigned char c) -> bool { return std::isalnum(c); };

	// Returns true for numerical characters only
	auto g_is_digit = [](unsigned char c) -> bool { return std::isdigit(c); };

	/**
	 *****************************************************************
	 *					Compilation step functions
	 *****************************************************************
	 */

	/**
	 * Preprocessor strips comments and extraneous data and checks for some syntax errors
	 */
	bool preprocess_source(std::string& source)
	{
		// strip comments
		size_t commentStart = std::string::npos;
		while ((commentStart = source.find("//")) != std::string::npos)
		{
			// Find the end of the line
			size_t lineEnd = source.find('\n', commentStart);
			if (lineEnd != std::string::npos)
			{
				// Erase the rest of the line after the comment
				source.erase(commentStart, lineEnd - commentStart);
			}
			else
			{
				// Erase to end of string
				source.erase(commentStart);
			}
		}

		// strip blocks
		size_t blockStart = std::string::npos;
		while ((blockStart = source.find("/*")) != std::string::npos)
		{
			// Find the end of the block
			size_t blockEnd = source.find("*/", blockStart);
			if (blockEnd == std::string::npos)
			{
				// If there's no matching block end, it's an error
				std::string line = get_full_line(source, blockStart);
				auto lineNum = get_line_num(source, blockStart);
				std::cerr << "Missing closing block started on line " << lineNum << "\nLine: " << line << std::endl;
				return false;
			}
			else
			{
				// Erase between the blocks
				// add 2 to erase count to include the block terminator
				source.erase(blockStart, (blockEnd - blockStart) + 2);
			}
		}

		// check for missing brackets or parentheses
		for (std::size_t i = 0; i < source.length(); ++i)
		{
			if (source[i] == '(')
			{
				auto pair = find_matching_parenthesis(source, i);
				if (pair == std::string::npos)
				{
					// No match for this opening paren found
					auto lineNum = get_line_num(source, i);
					auto line = get_full_line(source, i);

					std::cerr << "Missing closing parenthesis for opening parenthesis found on line " << lineNum <<
						": " << line << std::endl;
					return false;
				}
			}
			else if (source[i] == ')')
			{
				auto pair = find_reverse_matching_parenthesis(source, i);
				if (pair == std::string::npos)
				{
					// unexpected closing parenthesis found
					auto lineNum = get_line_num(source, i);
					auto line = get_full_line(source, i);

					std::cerr << "Unexpected character ')' on line " << lineNum << ": " << line << std::endl;
					return false;
				}
			}
			else if (source[i] == '{')
			{
				auto pair = find_matching_bracket(source, i);
				if (pair == std::string::npos)
				{
					// missing closing bracket somewhere
					auto lineNum = get_line_num(source, i);
					auto line = get_full_line(source, i);

					std::cerr << "Missing closing bracket for opening bracket found on line " << lineNum <<
						": " << line << std::endl;
					return false;
				}
			}
			else if (source[i] == '}')
			{
				auto pair = find_reverse_matching_bracket(source, i);
				if (pair == std::string::npos)
				{
					// unexpected closing bracket
					auto lineNum = get_line_num(source, i);
					auto line = get_full_line(source, i);

					std::cerr << "Unexpected character '}' on line " << lineNum << ": " << line << std::endl;
					return false;
				}
			}
		}

		return true;
	}

	/**
	 * This function parses an SGL function's name, return type, and parameters
	 */
	FunctionData parse_function_def(std::string& src)
	{
		FunctionData func;
		func.FunctionSource = src;
		func.FunctionName = "";

		// Find function's name
		// strip "func:" and any whitespace after it
		src.erase(0, 5);
		strip_leading_whitespace(src);

		// front of string should now be function identifier
		// quick check for illegal function identifier
		if (std::isdigit(src[0]))
		{
			std::cerr << "Function identifiers cannot begin with a digit!" << std::endl;
			return func;
		}

		// find end of identifier
		auto nameEnd = std::find_if_not(src.begin(), src.end(), g_is_alnum);
		if (nameEnd == src.end())
		{
			// if the entire rest of the string is alphanumeric, then where is the () {} and such? invalid
			std::cerr << "Function identifier '" << src << "' is invalid";
			return func;
		}

		// grab the function name substring
		std::string funcIdentifier = src.substr(0, nameEnd - src.begin());

		// Find parameters
		// Find opening of the parameter list
		auto paramStart = src.find('(');
		if (paramStart == std::string::npos)
		{
			// Missing () part of function declaration
			std::cerr << "Missing parameter list for function " << funcIdentifier << std::endl;
			return func;
		}

		// find end
		auto paramEnd = find_matching_parenthesis(src, paramStart);
		if (paramEnd == std::string::npos)
		{
			// Missing closing parenthesis
			std::cerr << "Missing closing parenthesis for function " << funcIdentifier << std::endl;
			return func;
		}

		// strip whitespace after param start
		strip_whitespace_at(src, paramStart + 1);
		// step forward so paramStart points to actual parameter instead of opening parenthesis
		++paramStart;

		// check if function has parameters
		if (paramEnd > paramStart)
		{
			// grab a substring that contains only the parameter list
			std::string paramList = src.substr(paramStart, paramEnd - paramStart);
			// find each parameter
			std::size_t lastParam = 0;
			while (lastParam != std::string::npos)
			{
				// find comma that separates params (add 1 to skip previous found comma)
				auto param = paramList.find(',', lastParam + 1);
				// grab string for this parameter
				auto paramStr = paramList.substr(lastParam, param - lastParam);
				// check if start is comma or whitespace
				while (is_whitespace(paramStr.front()) || paramStr.front() == ',')
				{
					paramStr.erase(0, 1);
				}

				// just in case some psycho has a space between the identifier and the next comma or parens...
				strip_tailing_whitespace(paramStr);

				// what's left now should be a type and an identifier separated by whitespace
				// left of whitespace = var type
				// right of whitespace = var identifier
				auto typeEnd = std::find_if_not(paramStr.begin(), paramStr.end(), g_is_alnum);
				if (typeEnd == paramStr.end())
				{
					// no space, therefore invalid
					std::cerr << "Missing identifier in function parameter" << std::endl;
					return func;
				}

				// paramStr.begin() -> typeEnd substr should be type
				auto typeLast = (typeEnd - paramStr.begin());
				auto typeStr = paramStr.substr(0, typeLast);

				// do the same for identifier, which should be typeLast -> paramStr.end()
				auto idStr = paramStr.substr(typeLast);
				// strip whitespace since it's probably there
				strip_leading_whitespace(idStr);

				// make sure type is valid
				TypeData type = get_type(typeStr);
				if (!type.is_valid())
				{
					std::cerr << "Unrecognized type " << typeStr << " in function declaration" << std::endl;
					return func;
				}
				else if (type == Types::void_type)
				{
					// void not allowed as anything except function return type
					std::cerr << "Illegal use of void type in function parameter" << std::endl;
					return func;
				}

				// Build param struct
				FunctionData::FunctionParam fparam;
				fparam.ParamName = idStr;
				fparam.ParamType = type;

				// Add to list
				func.FunctionParams.push_back(fparam);

				lastParam = param;
			}
		}

		// find return type
		// an SGL function return type clause comes after the end of the parameters and looks like "-> TYPE"
		// if there is no clause, the return type is void
		TypeData returnType = Types::void_type;
		auto clauseStart = src.find("->", paramEnd);
		if (clauseStart != std::string::npos)
		{
			// clause start points to the '-'
			// clause start + 1 points to the '>'
			// therefore clause start + 2 should be trimmable whitespace
			auto retTypeStart = clauseStart + 2;

			strip_whitespace_at(src, retTypeStart);

			// now clauseStart + 2 should be start of return type
			auto retTypeEnd = std::find_if_not(src.begin() + retTypeStart, src.end(), g_is_alnum);
			if (retTypeEnd == src.end())
			{
				// if we only found alphanumerics for the rest of the string, then there must be no function body
				std::cerr << "Invalid return type clause" << std::endl;
				return func;
			}

			// extract return type string
			auto retTypeEndChar = (retTypeEnd - src.begin());
			auto retTypeStr = src.substr(retTypeStart, retTypeEndChar - retTypeStart);

			// check if valid
			TypeData rtype = get_type(retTypeStr);
			if (!rtype.is_valid())
			{
				std::cerr << "Unrecognized type " << retTypeStr << " in function return clause" << std::endl;
				return func;
			}

			// assign it to the return type
			returnType = rtype;
		}

		func.FunctionName = funcIdentifier;
		func.ReturnType = returnType;

		// for testing:
		std::cout << "Found a function called " << func.FunctionName << " that returns " << func.ReturnType.TypeName
			<< " and takes " << func.FunctionParams.size() << " arguments." << std::endl;
		if (func.FunctionParams.size() > 0)
		{
			std::cout << "Function params are:" << std::endl;
			for (auto param : func.FunctionParams)
			{
				std::cout << param.ParamType.TypeName << " " << param.ParamName << std::endl;
			}
		}

		return func;
	}

	bool compile_function_body(FunctionData fn)
	{
		// grab a copy of the source
		std::string source = fn.FunctionSource;

		// trim off everything up to (and including) the bracket
		auto openingBracket = source.find('{');
		source.erase(0, openingBracket + 1);

		// trim off the ending bracket and newlines/whitespace
		source.pop_back();
		strip_tailing_if(source, g_is_newline_or_whitespace);
		strip_leading_if(source, g_is_newline_or_whitespace);

		// If the block is empty, it is only a valid function if its return type is void
		if (source.empty())
		{
			if (fn.ReturnType == Types::void_type)
			{
				return true;
			}
			else
			{
				std::cerr << "Missing return statement in function " << fn.FunctionName << std::endl;
				return false;
			}
		}

		// begin parsing statements
		bool isDone = false;
		while (!isDone)
		{
			// strip any leading whitespace and newlines
			strip_leading_if(source, g_is_newline_or_whitespace);

			auto endOfStatement = std::string::npos;

			// check if this is a special statement
			// "isalnum" check makes sure we don't falsely consider var or func names
			// that begin with the same letters as the logic flow
			if (str_starts_with(source, "if") && !std::isalnum(source[2]))
			{
				// If condition found
				// find condition clause
				auto conditionClauseStart = source.find('(');
				if (conditionClauseStart == std::string::npos)
				{
					// missing conditional open
					std::cerr << "Missing conditional clause after 'if' keyword" << std::endl;
					return false;
				}

				auto conditionClauseEnd = find_matching_parenthesis(source, conditionClauseStart);
				// increment clause start to exclude starting parenthesis
				++conditionClauseStart;
				// grab substring that represents the conditional
				auto conditionClauseStr = source.substr(conditionClauseStart, conditionClauseEnd - conditionClauseStart);
			}
			else if (str_starts_with(source, "for") && !std::isalnum(source[3]))
			{
				// For loop found
			}
			else if (str_starts_with(source, "while") && !std::isalnum(source[5]))
			{
				// while loop found
			}
			else if (str_starts_with(source, "return") && !std::isalnum(source[6]))
			{
				// return statement found
			}
			else
			{
				// should be a normal statement, which ends with ;
				endOfStatement = source.find(';');
				if (endOfStatement == std::string::npos)
				{
					// missing semicolon
					std::cerr << "Missing semicolon" << std::endl;
					return false;
				}

				auto statementStr = source.substr(0, ++endOfStatement);
			}

			source.erase(0, endOfStatement);

			isDone = source.empty();
		}

		return true;
	}

	bool compile_source(std::string source)
	{
		bool result = true;

		CompilerState state;

		// Run preprocessor
		result = preprocess_source(source);
		if (!result)
		{
			return false;
		}

		// Find all function declarations
		std::size_t lastFunc = 0;
		while (lastFunc != std::string::npos)
		{
			auto funcStart = source.find("func:", lastFunc);
			if (funcStart != std::string::npos)
			{
				// find opening bracket
				auto openBracket = source.find('{', funcStart);
				if (openBracket == std::string::npos)
				{
					// function with no body, error
					auto lineNum = get_line_num(source, funcStart);
					auto line = get_full_line(source,funcStart);
					std::cerr << "Function declared on line " << lineNum << ", but no function body was found.\nLine:"
						<< line << std::endl;
					return false;
				}

				// find closing bracket
				auto endBracket = find_matching_bracket(source, openBracket);
				if (endBracket == std::string::npos)
				{
					// function body missing closer
					// Can't return specific information since it's hard to say where the missing bracket occured
					std::cerr << "Missing bracket detected while parsing function declarations" << std::endl;
					return false;
				}

				auto funcSource = source.substr(funcStart, endBracket + 1);
				auto fn = parse_function_def(funcSource);
				if (!fn.is_valid())
				{
					return false;
				}

				state.Functions.push_back(fn);

				lastFunc = endBracket;
			}
			else
			{
				lastFunc = std::string::npos;
			}
		}

		// Now that function names, return types, and params are documented, we can compile each one
		// Doing the first part before compiling the bodies allows each function to call each other
		// without requiring them to be ordered some specific way
		for (auto fn : state.Functions)
		{
			result = compile_function_body(fn);
			if (!result)
			{
				return false;
			}
		}

		return true;
	}
}