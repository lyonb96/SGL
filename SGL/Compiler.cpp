#include "Compiler.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "Instructions.h"
#include "SGLTypes.h"

constexpr const std::size_t MAX_SIZE = std::numeric_limits<std::size_t>::max();

struct SGLOperator
{
	std::string Operator;
	int Precedence;
};

// Array of operators in order of lowest to highest precedence
std::vector<SGLOperator> SGL_ops = { { "=", 0 }, { "-", 1 }, { "+", 1 }, { "%", 2 }, { "/", 2 }, {"*", 2} };

struct VariableState
{
	std::string VariableIdentifier = "";
	SGLType VariableType;

	/**
	 * Returns true if this VariableState slot is being used
	 */
	bool IsUsed() { return !VariableIdentifier.empty(); }

	/**
	 * Sets this VariableState slot to unused
	 */
	void SetUnused() { VariableIdentifier.clear(); }
};

/**
 * Holds info on the compiler's current state
 */
struct CompilerState
{
	std::vector<VariableState> Variables;

	/**
	 * Prepares the compiler for a new run
	 */
	void Prepare()
	{
		Variables.clear();
		for (auto i = 0; i < 10; ++i)
		{
			// fill with 10 preset variable slots
			Variables.push_back(VariableState());
		}
	}

	std::size_t GetAvailableVariableSlot()
	{
		// Find an unused slot
		for (std::size_t i = 0; i < Variables.size(); ++i)
		{
			if (!Variables[i].IsUsed())
			{
				return i;
			}
		}

		// If all are in use, add a new one and return its index
		Variables.push_back(VariableState());
		return Variables.size() - 1;
	}

	/**
	 * Returns the slot that the requested identifier is stored at
	 * Can also return size_t's max value if no var is found with that identifier
	 */
	std::size_t GetSlotForIdentifier(const std::string& id)
	{
		std::size_t slot = 0;
		for (slot = 0; slot < Variables.size(); ++slot)
		{
			if (Variables[slot].VariableIdentifier == id)
			{
				return slot;
			}
		}

		return MAX_SIZE;
	}
};

CompilerState SGL_CompilerState;

/**
 * Returns true if the character is considered whitespace
 */
bool is_whitespace(const char in)
{
	return in == ' ' || in == '\t';
}

/**
 * Returns true if the character is a newline character
 */
bool is_newline(const char in)
{
	return in == '\n' || in == '\r';
}

/**
 * Returns true if the character is valid to be used in an identifier (either function or variable)
 */
bool is_valid_character(const char in)
{
	return std::isalnum(in) || in == '_';
}

/**
 * Returns true if the entire string is alphanumeric
 */
bool is_alphanumeric(const std::string& in)
{
	return std::count_if(in.begin(), in.end(), [] (unsigned char c) -> bool { return std::isalnum(c) || c == '_'; }) == in.length();
}

/**
 * Returns true if the string is an integer literal
 */
bool is_str_int(const std::string& in)
{
	// check if it starts with a sign, if so skip the sign to measure the rest
	auto it = in.begin();
	auto reqLen = in.length(); // required count from count_if to return true
	if (*it == '-')
	{
		// starts with a sign, so skip the sign
		++it;
		--reqLen;
	}

	return std::count_if(it, in.end(), [](unsigned char c) -> bool { return std::isdigit(c); }) == reqLen;
}

/**
 * Returns true if the string is a float literal
 */
bool is_str_float(const std::string& in)
{
	// Check for sign
	bool isNegative = in.front() == '-';

	// Check if it ends with Ff
	auto end = in.back();
	if (end == 'f' || end == 'F')
	{
		// If the string ends with Ff, and the rest is numbers (or .), it is a float literal
		auto first = isNegative ? in.begin() + 1 : in.begin();
		auto last = in.end() - 1;
		auto count = std::count_if(first, last, [](unsigned char c) -> bool { return std::isdigit(c) || c == '.'; });

		return count == (last - first);
	}
	else
	{
		// if the string doesn't end with Ff, then it has to contain a decimal tobe a float
		auto dec = in.find('.');
		if (dec == std::string::npos)
		{
			return false;
		}

		// if it contains a decimal, the rest has to be numbers only
		auto first = isNegative ? in.begin() + 1 : in.begin();
		auto last = in.end();
		auto count = std::count_if(first, last, [](unsigned char c) -> bool { return std::isdigit(c) || c == '.'; });

		return count == (last - first);
	}
}

/**
 * Returns true if the string is a bool literal
 * "true" or "false"
 */
bool is_str_bool(const std::string& in)
{
	return (in == "true" || in == "false");
}

/**
 * Removes all whitespace from the front of the string
 */
void strip_leading_whitespace(std::string& in)
{
	while (!in.empty() && is_whitespace(in.front()))
	{
		in.erase(0, 1);
	}
}

/**
 * Removes all whitespace from the end of the string
 */
void strip_tailing_whitespace(std::string& in)
{
	while (!in.empty() && is_whitespace(in.back()))
	{
		in.pop_back();
	}
}

/**
 * Returns true if the given index of the given string is inside any parentheses
 */
bool is_in_parentheses(const std::string& str, std::size_t i)
{
	std::size_t openParens = 0;
	std::size_t pos = 0;
	while (pos < i && pos < str.length())
	{
		if (str[pos] == '(')
		{
			++openParens;
		}
		else if (str[pos] == ')')
		{
			--openParens;
		}
		++pos;
	}

	return (openParens > 0);
}

// helper types and functions for intermediate compilation steps

struct VariableDeclaration
{
	SGLType Type; // type
	std::string Identifier; // variable identifier
	std::string Value; // assigned value at creation, if any
	SGLResult Result; // result of parsing the variable declaration
};

VariableDeclaration parse_variable(std::string& line)
{
	VariableDeclaration decl;
	decl.Identifier = "";
	decl.Value = "";
	decl.Result = SGLResult::SGL_OK;

	size_t typeEnd = line.find(' ', 0);
	if (typeEnd == std::string::npos)
	{
		// If there is no space after the initial part of the line, this is a compilation error
		// as every variable declaration requires a type AND an identifier.
		decl.Result = SGLResult::SGL_ERR_UNEXPECTED_END_OF_LINE;
		return decl;
	}

	// otherwise, the substring from 0 to typeEnd is the type
	std::string typeStr = line.substr(0, typeEnd);
	//decl.Type = typeStr;

	// Check to verify that this type exists
	if (!is_type_registered(typeStr))
	{
		std::cout << "Error: unknown type " << typeStr << std::endl;
		decl.Result = SGLResult::SGL_ERR_UNKNOWN_TYPE;
		return decl;
	}

	decl.Type = get_types()[typeStr];

	// erase the type from the line now that it's parsed
	line.erase(0, typeEnd);
	// strip leading whitespace so the line starts with the variable identifier
	strip_leading_whitespace(line);

	// Now the next part should be the identifier
	// Note that the identifier must be alphanumeric
	size_t identifierStart = 0;
	size_t identifierEnd = identifierStart;
	while (is_valid_character(line[identifierEnd]))
	{
		++identifierEnd;
	}

	std::string identStr = line.substr(identifierStart, identifierEnd - identifierStart);
	decl.Identifier = identStr;

	// Now check for a value assignment at the end
	size_t assignPos = line.find('=');
	if (assignPos != std::string::npos)
	{
		// there is an assignment, so everything between the = and the ; is the assignment value
		std::string assignVal = line.substr(assignPos + 1);
		strip_leading_whitespace(assignVal);
		decl.Value = assignVal;
	}

	return decl;
}

/**
 * parse_statement v1 helped me realize quite a few things about parsing statements
 */
SGLResult parse_statement(std::string statement)
{
	/**
	 * Below are some sample SGL statements to parse
	 *
	 * int32 my_variable; <- Creates an int32 called "my_variable" with default value of 0 (implicit '=0')
	 * int32 my_other_var = 88; <- Creates another int32 with a value of 88
	 * int32 another_var = my_variable + my_other_var; <- adds two variables and stores the result in a new one
	 * int32 complex = ((i * 4) / x) + (y + j); <- nested parens
	 * call_some_function(complex); <- calls a function with an argument
	 * if (my_bool) { ... } <- if condition, will require special parsing since it will have multiple statements inside
	 *
	 * Easiest distinction to make early on is to find operations and work on those by figuring out their left and right
	 * arguments. If there are parentheses, step inside the parentheses and parse there first. One major challenge I foresee
	 * is differentiating between a function call and a nested parenthetical operation. To work around this, I should search
	 * the statement for function calls, emit instructions to do the function first, and then do the rest. Since this may cause
	 * out of order instruction parsing, 
	 */

	//std::cout << "Parsing statement:" << statement << std::endl;

	//// find parens
	//bool allParensFound = false;
	//std::size_t lastParens = 0;
	//while (!allParensFound)
	//{
	//	std::size_t pStart = statement.find_first_of('(', lastParens);
	//	if (pStart != std::string::npos)
	//	{
	//		// find matching close parens
	//		int openParens = 1;
	//		std::size_t i = pStart + 1;
	//		while (openParens != 0 && i < statement.length())
	//		{
	//			if (statement[i] == '(')
	//			{
	//				++openParens;
	//			}
	//			else if (statement[i] == ')')
	//			{
	//				--openParens;
	//			}

	//			if (openParens != 0)
	//			{
	//				++i;
	//			}
	//		}

	//		if (openParens != 0)
	//		{
	//			// open parens should be 0, otherwise there are missing parentheses
	//			return SGLResult::SGL_ERR_MISSING_PARENTHESES;
	//		}

	//		lastParens = i;

	//		auto parenString = statement.substr(pStart + 1, i - pStart - 1);
	//		auto result = parse_statement(parenString);
	//		if (result != SGLResult::SGL_OK)
	//		{
	//			return result;
	//		}

	//		// insert special character denoting that this operand is parsed and on the stack
	//		statement.replace(pStart, i - pStart + 1, 1, '^');
	//		// adjust last parens to point to the end of the special char
	//		lastParens = pStart + 1;
	//	}
	//	else
	//	{
	//		allParensFound = true;
	//	}
	//}

	//// TODO: operator precedence
	//std::size_t c = 0;
	//while (c < statement.length())
	//{
	//	auto it = std::find(OP_CHARS.begin(), OP_CHARS.end(), statement[c]);
	//	if (it != OP_CHARS.end())
	//	{
	//		break;
	//	}
	//	++c;
	//}

	//if (c == statement.length())
	//{
	//	return SGLResult::SGL_ERR_UNEXPECTED_END_OF_LINE;
	//}

	//std::cout << "statement: " << statement << " operation found: " << statement[c] << std::endl;

	//// find left hand operand
	//// somewhere left of c should be the first operand
	//// strip whitespace left of operator
	//while (c != 0 && is_whitespace(statement[c - 1]))
	//{
	//	// erase whitespace characters
	//	statement.erase(c - 1, 1);
	//	--c;
	//}

	//if (c == 0)
	//{
	//	// missing left operand
	//	return SGLResult::SGL_ERR_MISSING_LEFT_OPERAND;
	//}

	//// now everything between the operator and the next space should be the left operand
	//std::size_t leftStart = c - 1;
	//for (; leftStart > 0 && !is_whitespace(statement[leftStart]); --leftStart);

	//// get the operand as a string
	//std::string leftOp = statement.substr(leftStart, c - leftStart);

	//// now do the same for the right operand
	//// trim whitespace
	//for (; c < statement.length() && is_whitespace(statement[c + 1]); statement.erase(c + 1, 1));

	//if (c == statement.length() - 1)
	//{
	//	// missing right operand
	//	return SGLResult::SGL_ERR_MISSING_RIGHT_OPERAND;
	//}

	//// find where the right operand ends
	//std::size_t rightEnd = c + 1;
	//for (; rightEnd < statement.length() && !is_whitespace(statement[rightEnd]); ++rightEnd);

	//// get the operand as a string
	//std::string rightOp = statement.substr(c + 1, rightEnd - c);

	//// the operands can be one of 3 things:
	//// a variable
	//// a constant
	//// a special character denoting that the value needed is already on the stack
	//
	//// first check for special character

	
	return SGLResult::SGL_OK;
}

struct ExpressionResult
{
	bool Success;
	SGLType ResultType;
	std::size_t VarSlot;
};

/**
 * Special function for parsing the left operand of an assignment operator
 * This side of the function should always be a variable, so the return is
 * the variable slot the var is in, or MAX_SIZE if it's not a variable
 */
std::size_t parse_assignment_left(std::string expr)
{
	// The only thing this can be is either an existing variable or a new variable declaration
	// if it's a new variable, there will be whitespace, so check for that first
	bool hasWhitespace = false;
	for (auto c : expr)
	{
		if (is_whitespace(c))
		{
			hasWhitespace = true;
			break;
		}
	}

	if (hasWhitespace)
	{
		// parse variable declaration
		VariableDeclaration decl = parse_variable(expr);
		if (decl.Result != SGLResult::SGL_OK)
		{
			return MAX_SIZE;
		}

		std::size_t slot = SGL_CompilerState.GetAvailableVariableSlot();
		SGL_CompilerState.Variables[slot].VariableType = decl.Type;
		SGL_CompilerState.Variables[slot].VariableIdentifier = decl.Identifier;

		return slot;
	}
	else
	{
		//expr should be an existing variable
		std::size_t slot = SGL_CompilerState.GetSlotForIdentifier(expr);

		return slot;
	}
}

ExpressionResult parse_expression(std::string expr)
{
	/**
	 * The algorithm:
	 * 
	 * 0) Before anything else happens, find function calls and parse them first
	 * 1) Strip parentheses surrounding whole expression, if present
	 * 2) Find the lowest-precedence operator that is not in parentheses
	 * 3a) If operator is found, split into left and right operand and recursively parse them (left then right)
	 * 3b) Emit instruction(s) to execute the operator that was found
	 * 4a) If no operator is found, the operand must be either a constant or variable
	 * 4b) Emit instruction(s) to load the constant or variable to the stack
	 */

	//std::cout << "Parsing expression: " << expr << std::endl;

	// prepare result struct
	ExpressionResult result;
	result.Success = true;
	result.VarSlot = MAX_SIZE;

	// first, some pre-work
	strip_leading_whitespace(expr);
	strip_tailing_whitespace(expr);
	if (expr.back() == ';') expr.pop_back();

	// step zero - find and parse function calls before anything else
	// this will be implemented at a later time, but the gist will be to
	// detect function calls, find the arg list, call parse_expression on each arg,
	// and then call the function. Store the result to a temporary variable, and do
	// expr.replace() to replace the function call with the identifier of the temp
	// variable. This ensures that all function calls are handled before anything
	// else, and eliminates any possible ambiguity between precedence-modifying
	// parantheses and function argument lists

	// step one - check if expression is wrapped in parentheses
	while (expr.front() == '(' && expr.back() == ')')
	{
		// funny story... just because it starts and ends with parens doesn't mean it's all in parens
		// For example: (x + 5) / (y * 3) starts with ( and ends with ) but contains an op outside of
		// those parentheses. Naively checking if it only starts and ends with () results in a syntax
		// error when we strip the parentheses, as we're left with "x + 5) / (y * 3"
		// so we need to verify that all chars of string are in parens before erasing
		bool doErase = true;
		for (std::size_t i = 1; i < expr.length() - 1; ++i)
		{
			if (!is_in_parentheses(expr, i))
			{
				// if any character is out of parens, don't erase the surrounding parens
				doErase = false;
				break;
			}
		}

		if (doErase)
		{
			// all chars in parentheses, do erase
			expr.erase(0, 1); // remove (
			expr.pop_back(); // remove )
		}
		else
		{
			// break and don't erase if any are out of parens
			break;
		}
	}

	// step two - find the lowest-precedence operator that is not in parentheses
	// if two operators are found with the same precedence, the one further right is selected first
	std::size_t opPos = std::string::npos;
	SGLOperator foundOp = { "", 999 };
	for (auto op : SGL_ops)
	{
		if (op.Precedence > foundOp.Precedence)
		{
			break;
		}

		// check if expression contains operator
		std::size_t thisOpPos = expr.find(op.Operator);
		if (thisOpPos == std::string::npos)
		{
			continue;
		}

		// verify the operator is not in parens
		if (is_in_parentheses(expr, thisOpPos))
		{
			continue;
		}
		else
		{
			if (op.Precedence < foundOp.Precedence)
			{
				// if this operator's precedence is lower than what we have now, store it without question
				foundOp = op;
				opPos = thisOpPos;
			}
			else if (op.Precedence == foundOp.Precedence && opPos < thisOpPos)
			{
				// possible second case, if this operator's precedence is equal, but comes up later in the string
				// we'll use it instead (to ensure left-to-right parsing)
				foundOp = op;
				opPos = thisOpPos;
			}
		}
	}

	// Check if there is an operator or if this is something else
	if (opPos != std::string::npos)
	{
		// step 3a - operator found, split into left and right and recursively parse
		// note that some operators, notably assignment (=) and increment/decrement
		// will have special handling here

		std::string leftOp = expr.substr(0, opPos);
		std::string rightOp = expr.substr(opPos + 1);

		if (foundOp.Operator == "=")
		{
			auto leftSlot = parse_assignment_left(leftOp);
			if (leftSlot == MAX_SIZE)
			{
				result.Success = false;
				return result;
			}

			auto rightResult = parse_expression(rightOp);
			if (!rightResult.Success)
			{
				result.Success = false;
				return result;
			}

			SGLType leftType = SGL_CompilerState.Variables[leftSlot].VariableType;
			SGLType rightType = rightResult.ResultType;

			if (leftType.TypeName != rightType.TypeName)
			{
				// need to cast right side
				SGLInstruction cast = get_cast_instruction(rightType, leftType);
			}

			std::cout << "INT_STORE " << leftSlot << std::endl;
		}
		else
		{
			auto leftResult = parse_expression(leftOp);

			if (!leftResult.Success)
			{
				result.Success = false;
				return result;
			}

			auto rightResult = parse_expression(rightOp);

			if (!rightResult.Success)
			{
				result.Success = false;
				return result;
			}

			if (foundOp.Operator == "=")
			{
				//// assignment is special
				//// The left expression result has to have a stored variable slot
				//// otherwise the left operand isn't a variable and thus cannot be assigned to
				//if (leftResult.VarSlot == MAX_SIZE)
				//{
				//	// no variable in left operand, therefore failure
				//	result.Success = false;
				//	result.ResultType = get_types()["void"];
	
				//	return result;
				//}

				//result.ResultType = leftResult.ResultType;
				//result.VarSlot = MAX_SIZE;
				//result.ResultType = get_types()["void"];
				//std::cout << "INT_STORE " << result.VarSlot << std::endl;
	
				//return result;
			}
			else
			{
				// check if the types on either side are equal
				if (leftResult.ResultType.TypeName != rightResult.ResultType.TypeName)
				{
					// If types are not the same, the right operand needs to be cast to the left operand if possible
					SGLType leftType = leftResult.ResultType;
					SGLType rightType = rightResult.ResultType;

					// if either type is void, it's an illegal operand
					if (leftType.TypeSize == 0 || rightType.TypeSize == 0)
					{
						result.Success = false;
						return result;
					}

					// get cast instruction needed
					SGLInstruction cast = get_cast_instruction(rightType, leftType);
					
					// emit cast instruction
				}

				result.ResultType = leftResult.ResultType;

				// step 3b - emit instruction for the operation
				if (foundOp.Operator == "*")
				{
					std::cout << "INT_MUL" << std::endl;
				}
				else if (foundOp.Operator == "+")
				{
					std::cout << "INT_ADD" << std::endl;
				}
				else if (foundOp.Operator == "-")
				{
					std::cout << "INT_SUB" << std::endl;
				}
				else if (foundOp.Operator == "/")
				{
					std::cout << "INT_DIV" << std::endl;
				}
				else if (foundOp.Operator == "%")
				{
					std::cout << "INT_MOD" << std::endl;
				}
			}
		}
		return result;
	}
	else
	{
		// step 4a - no operator found
		// This means that "expr" can be one of four things
		// 1 - a variable declaration (such as int32 i)
		// 2 - a variable reference (such as i)
		// 3 - a constant (such as 4, 18F, false, etc)

		// let's check for each one
		// constants and variable references cannot have whitespace in them, so we'll check for that first
		bool hasWhitespace = false;
		std::size_t whitespacePos = 0;
		for (auto c : expr)
		{
			if (is_whitespace(c))
			{
				hasWhitespace = true;
				break;
			}
			++whitespacePos;
		}

		if (hasWhitespace)
		{
			// the only thing this expression can legally be now is a variable declaration
			VariableDeclaration varDecl = parse_variable(expr);
			if (varDecl.Result != SGLResult::SGL_OK)
			{
				std::cerr << "Failed to parse expression " << expr << std::endl;
				result.Success = false;
				return result;
			}
			
			// make sure this isn't a redeclaration of an existing variable
			if (SGL_CompilerState.GetSlotForIdentifier(varDecl.Identifier) != MAX_SIZE)
			{
				std::cerr << "Cannot declare two variables with the same identifier!" << std::endl;
				result.Success = false;
				return result;
			}

			// if all is well, push this to the variable list
			std::size_t varPos = SGL_CompilerState.GetAvailableVariableSlot();
			SGL_CompilerState.Variables[varPos] = { varDecl.Identifier, varDecl.Type };

			result.Success = true;
			result.VarSlot = varPos;
			result.ResultType = varDecl.Type;

			return result;
		}
		else
		{
			// step 4b - emit instruction(s) to load the constant or variable

			// expression must be constant or existing variable name
			auto slot = SGL_CompilerState.GetSlotForIdentifier(expr);
			if (slot != MAX_SIZE)
			{
				// variable found
				// emit instruction to load variable
				// for now, int is supported only
				std::cout << "INT_LOAD " << slot << std::endl;
				result.ResultType = SGL_CompilerState.Variables[slot].VariableType;
				result.VarSlot = slot;
				return result;
			}
			else
			{
				// now it has to be a constant, otherwise syntax error
				if (is_str_int(expr))
				{
					int value = std::stoi(expr, nullptr, 0);
					std::cout << "INT_CONST " << value << std::endl;

					result.ResultType = get_types()["int32"];
					return result;
				}
				else
				{
					std::cerr << "Not sure what " << expr << " is..." << std::endl;
					result.Success = false;
					return result;
				}
			}
		}
	}
}

SGLResult compile_sgl_function(std::string& source)
{
	// strip the "func:" keyword off the top
	source.erase(0, 5);
	// strip any whitespace between "func:" and the function identifier
	strip_leading_whitespace(source);
	// strip whitespace off the end
	strip_tailing_whitespace(source);

	// find parens
	size_t parensStart = source.find_first_of('(');
	if (parensStart == std::string::npos)
	{
		// No parens found
		return SGLResult::SGL_ERR_MISSING_PARENTHESES;
	}

	// Break out the function name
	std::string funcName = source.substr(0, parensStart);
	std::cout << "Function name: |" << funcName << std::endl;

	// ensure the function name is alphanumeric
	if (!is_alphanumeric(funcName))
	{
		return SGLResult::SGL_ERR_INVALID_IDENTIFIER;
	}

	return SGLResult::SGL_OK;
}

SGLResult compile_sgl(std::string source)
{
	// Check for invalid length
	if (source.length() == 0)
	{
		return SGLResult::SGL_ERR_SOURCE_INVALID;
	}

	/**
	 * Preprocessing - strip line and block comments and remove all newlines
	 */

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
			return SGLResult::SGL_ERR_UNCLOSED_BLOCK_COMMENT;
		}
		else
		{
			// Erase between the blocks
			// add 2 to erase count to include the block terminator
			source.erase(blockStart, (blockEnd - blockStart) + 2);
		}
	}

	// strip line endings
	for (size_t i = 0; i < source.length();)
	{
		if (is_newline(source[i]))
		{
			source.erase(i, 1);
		}
		else
		{
			++i;
		}
	}

	// strip tailing whitespace
	strip_tailing_whitespace(source);

	// For testing, dump the preprocessed SGL script to the console
#ifdef _DEBUG
	//std::cout << "Preprocessing complete:\n" << source << std::endl;
#endif

	/**
	 * Compilation loop
	 */

	bool isDone = false;
	while (!isDone)
	{
		// This is the "outermost" compilation layer, AKA the lowest scope
		// No instructional code can occur here, only functions and global variables can exist here
		// If anything else is found, it is a compilation error

		// strip the leading whitespace for the current position
		strip_leading_whitespace(source);

		if (source.rfind("func:", 0) == 0)
		{
			// first, check if we're starting with a function
			// If it's a function, we need to find where it ends, so we need to find the matching curly braces
			// my idea for the algorithm is to do a ++ for each bracket open and a -- for each bracket close
			// once that's equal to 0, we should be back to the end of the function

			// find where the bracket starts
			size_t opening = source.find_first_of('{');
			if (opening == std::string::npos)
			{
				// Compile error, missing open bracket
				return SGLResult::SGL_ERR_MISSING_CURLY_BRACE;
			}

			// find the matching end bracket
			int openBrackets = 1;
			size_t closing = std::string::npos;
			size_t currentPos = opening + 1;
			while (openBrackets != 0 && currentPos < source.length())
			{
				if (source[currentPos] == '{')
				{
					++openBrackets;
				}
				else if (source[currentPos] == '}')
				{
					--openBrackets;
				}
				++currentPos;
			}

			// verify that something was found
			if (openBrackets > 0)
			{
				// no closing bracket found
				return SGLResult::SGL_ERR_MISSING_CURLY_BRACE;
			}

			// closing should be at the ending bracket
			closing = currentPos;

			// substring from start to closing is the whole function body
			std::string function = source.substr(0, closing);

			SGLResult res = compile_sgl_function(function);
			if (res != SGLResult::SGL_OK)
			{
				return res;
			}

			// remove function from parsed source
			source.erase(0, closing);
		}
		else
		{
			// if it's not a function, it has to be a global variable declaration
			// so a substring from the current parsing position to the very next semicolon should be a var decl
			size_t endOfStatement = source.find(';', 0);
			if (endOfStatement == std::string::npos)
			{
				return SGLResult::SGL_ERR_MISSING_SEMICOLON;
			}

			// extract the statement
			std::string varStatement = source.substr(0, endOfStatement);
			VariableDeclaration var = parse_variable(varStatement);
			if (var.Result != SGLResult::SGL_OK)
			{
				return var.Result;
			}

			// now erase the line
			source.erase(0, endOfStatement + 1);

			std::cout << "Variable declaration, type=\"" << var.Type.TypeName << "\" id=\"" << var.Identifier << "\" val=\"" << var.Value << "\"" << std::endl;
			auto pos = SGL_CompilerState.GetAvailableVariableSlot();
			SGL_CompilerState.Variables[pos].VariableIdentifier = var.Identifier;
			SGL_CompilerState.Variables[pos].VariableType = var.Type;
		}

		if (source.empty())
		{
			isDone = true;
		}
	}

	return SGLResult::SGL_OK;
}

#define TEST_MACRO(FN, STR_TO_TEST, EXPECTED) std::cout << "\t" #FN "(\"" STR_TO_TEST "\") Expected: " #EXPECTED ". Actual: " << FN##(STR_TO_TEST) << std::endl;
#define PARENS_TEST(STR, I) std::cout << "\tis_in_parentheses(\"" STR "\", " << I << "): " << is_in_parentheses(STR, I) << std::endl

void execute_compiler_test()
{
	std::cout << "---------------- SGL Compiler function tests ----------------" << std::endl;
	std::cout << "testing is_str_int():" << std::endl;
	TEST_MACRO(is_str_int, "This is not an int.", 0);
	TEST_MACRO(is_str_int, "48", 1);
	TEST_MACRO(is_str_int, "-5802351245", 1);
	TEST_MACRO(is_str_int, "1 2 3 4", 0);

	std::cout << "testing is_str_float():" << std::endl;
	TEST_MACRO(is_str_float, "12345.0F", 1);
	TEST_MACRO(is_str_float, "-58F", 1);
	TEST_MACRO(is_str_float, "-34234234.", 1);
	TEST_MACRO(is_str_float, "122", 0);
	TEST_MACRO(is_str_float, "This is a long string with numbers (123) that ends with f", 0);

	std::cout << "testing is_in_parentheses():" << std::endl;
	PARENS_TEST("Some words", 5);
	PARENS_TEST("(Hello, world!)", 4);
	PARENS_TEST("complicated! (a)", 15);
	PARENS_TEST("()()() () (((((egg))))) egg ()() ()", 16); // 1
	PARENS_TEST("()()() () (((((egg))))) egg ()() ()", 25); // 0

	std::cout << "Testing expression parsing:" << std::endl;
	parse_expression("int32 x = 5;");
	parse_expression("int32 y = 12;");
	parse_expression("int32 z = 6;");
	parse_expression("int32 w = 8;");
	parse_expression("int32 i = 10 * (w + z * (8 * x)) % y / (x + 1);"); // complex nested parens
	//parse_statement("i = (((x + 5) * (y / 3)) + 50) + (z * 2)"); // complex nested
	//parse_statement("int32 i = ((((x + 5) * y) / z) + w)"); // simple nested
	//parse_statement("float x = 5;"); // simple assignment
	//parse_statement("int32 b = 5 * x + 10;"); // multiple ops with no parens
	//parse_statement("words and such"); // should fail

	std::cout << "---------------- SGL Compiler tests complete ----------------" << std::endl;
}