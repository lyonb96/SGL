#include "Compiler.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "StringHelpers.h"

namespace SGL
{
	/**
	 * Struct that holds information about types in SGL
	 */
	struct TypeData
	{
		std::string TypeName;
	};

	namespace Types
	{
		TypeData int_type = { "int32" };
		TypeData float_type = { "float" };
		TypeData void_type = { "void" };
	}

	std::vector<TypeData> g_types = { Types::int_type, Types::float_type, Types::void_type };

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
	};

	/**
	 * Preprocessor strips comments and extraneous data
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
	}

	bool parse_function_def(std::string& src)
	{
		FunctionData func;
		func.FunctionSource = src;

		// Find function's name
		// strip "func:" and any whitespace after it
		src.erase(0, 5);
		strip_leading_whitespace(src);

		// front of string should now be function identifier
		// quick check for illegal function identifier
		if (std::isdigit(src[0]))
		{
			std::cerr << "Function identifiers cannot begin with a digit!" << std::endl;
			return false;
		}

		// find end of identifier
		auto nameEnd = std::find_if_not(src.begin(), src.end(), [](unsigned char c) -> bool { return isalnum(c); });
		if (nameEnd == src.end())
		{
			// if the entire rest of the string is alphanumeric, then where is the () {} and such? invalid
			std::cerr << "Function identifier '" << src << "' is invalid";
			return false;
		}

		// grab the function name substring and store it
		std::string funcIdentifier = src.substr(0, nameEnd - src.begin());
		func.FunctionName = funcIdentifier;

		// Find parameters
		// Find opening of the parameter list
		auto paramStart = src.find('(');
		if (paramStart == std::string::npos)
		{
			// Missing () part of function declaration
			std::cerr << "Missing parameter list for function " << funcIdentifier << std::endl;
			return false;
		}

		// find end
		auto paramEnd = find_matching_parenthesis(src, paramStart);
		if (paramEnd == std::string::npos)
		{
			// Missing closing parenthesis
			std::cerr << "Missing closing parenthesis for function " << funcIdentifier << std::endl;
			return false;
		}

		// strip whitespace after param start
		strip_whitespace_at(src, paramStart + 1);

		// check if function has parameters
		if (paramEnd > paramStart + 1)
		{
			// grab a substring that contains only the parameter list
			std::string paramList = src.substr(paramStart + 1, paramEnd - paramStart);
			// find each parameter
			std::size_t lastParam = 0;
			while (lastParam != std::string::npos)
			{
				// find comma that separates params
				auto param = paramList.find(',');
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

				lastParam = param;
			}
		}

		return true;
	}

	bool compile_source(std::string source)
	{
		bool result = true;

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

				auto funcSource = source.substr(funcStart, endBracket);
				result = parse_function_def(funcSource);
				if (!result)
				{
					return false;
				}
			}

			lastFunc = funcStart;
		}
	}
}