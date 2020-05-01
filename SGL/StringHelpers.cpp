#include "StringHelpers.h"

#include <algorithm>

namespace SGL
{
	std::size_t find_pair(const std::string& str, char open, char close, std::size_t first)
	{
		std::size_t closePos = std::string::npos;
		auto counter = 0;

		for (size_t i = first; i < str.length(); ++i)
		{
			if (str[i] == open)
			{
				++counter;
			}
			else if (str[i] == close)
			{
				--counter;
			}

			if (counter == 0)
			{
				closePos = i;
				break;
			}
		}

		return closePos;
	}

	std::size_t find_matching_parenthesis(const std::string& str, std::size_t firstPar)
	{
		return find_pair(str, '(', ')', firstPar);
	}

	std::size_t find_matching_bracket(const std::string& str, std::size_t firstBracket)
	{
		return find_pair(str, '{', '}', firstBracket);
	}

	std::string get_full_line(const std::string& src, std::size_t pos)
	{
		if (pos >= src.length())
		{
			return "";
		}

		std::size_t beginning = src.rfind('\n', pos);
		std::size_t end = src.find('\n', pos);

		return src.substr(beginning, (end - beginning));
	}

	std::size_t get_line_num(const std::string& src, std::size_t pos)
	{
		std::size_t lineNum = 1;

		// clamp pos to string length
		pos = pos >= src.length() ? src.length() - 1 : pos;

		for (std::size_t i = 0; i < pos; ++i)
		{
			if (src[i] == '\n')
			{
				++lineNum;
			}
		}

		return lineNum;
	}

	bool is_whitespace(const char in)
	{
		return in == ' ' || in == '\t';
	}

	bool is_newline(const char in)
	{
		return in == '\n' || in == '\r';
	}

	bool is_valid_character(const char in)
	{
		return std::isalnum(in) || in == '_';
	}

	bool is_alphanumeric(const std::string& in)
	{
		return std::count_if(in.begin(), in.end(), [](unsigned char c) -> bool { return std::isalnum(c) || c == '_'; }) == in.length();
	}

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

	bool is_str_bool(const std::string& in)
	{
		return (in == "true" || in == "false");
	}

	void strip_leading_whitespace(std::string& in)
	{
		while (!in.empty() && is_whitespace(in.front()))
		{
			in.erase(0, 1);
		}
	}

	void strip_tailing_whitespace(std::string& in)
	{
		while (!in.empty() && is_whitespace(in.back()))
		{
			in.pop_back();
		}
	}

	void strip_whitespace_at(std::string& in, std::size_t pos)
	{
		while (pos < in.length() && is_whitespace(in[pos]))
		{
			in.erase(pos, 1);
		}
	}

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
}