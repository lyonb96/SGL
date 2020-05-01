#pragma once

#include <string>

/**
 * A handful of useful functions for string manipulation and querying
 */

namespace SGL
{
    /**
     * Finds a matching pair of two characters
     * Used to find two symbols that encapsulate an area but may include nested versions of the same symbols
     * Examples include (), {}, "", '', etc.
     *
     * Algorithm:
     * Check each character beginning at 'first.'
     * For each instance of 'open' found, a counter is increased.
     * For each instance of 'close' found, the counter is decreased
     * When the counter reaches 0, the character at the current position is the matching closer
     * If the end of the string is found and counter > 0, the function returns std::string::npos
     */
    std::size_t find_pair(const std::string& str, char open, char close, std::size_t first);

    /**
     * Finds the closing parenthesis that goes with an opening parenthesis.
     * firstPar should be the index of the first parenthesis to find a match for
     * Returns std::string::npos if no match is found
     */
    std::size_t find_matching_parenthesis(const std::string& str, std::size_t firstPar);

    /**
     * Finds the closing bracket that goes with an opening bracket.
     * firstPar should be the index of the first bracket to find a match for
     * Returns std::string::npos if no match is found
     */
    std::size_t find_matching_bracket(const std::string& str, std::size_t firstBracket);

    /**
     * Returns a substring of the given source that encapsulates the full line of a character
     */
    std::string get_full_line(const std::string& src, std::size_t pos);

    /**
     * Returns the line number that the given index lands on
     */
    std::size_t get_line_num(const std::string& src, std::size_t pos);

    /**
     * Returns true if the character is considered whitespace
     */
    bool is_whitespace(const char in);

    /**
     * Returns true if the character is a newline character
     */
    bool is_newline(const char in);

    /**
     * Returns true if the character is valid to be used in an identifier (either function or variable)
     */
    bool is_valid_character(const char in);

    /**
     * Returns true if the entire string is alphanumeric
     */
    bool is_alphanumeric(const std::string& in);

    /**
     * Returns true if the string is an integer literal
     */
    bool is_str_int(const std::string& in);

    /**
     * Returns true if the string is a float literal
     */
    bool is_str_float(const std::string& in);

    /**
     * Returns true if the string is a bool literal
     * "true" or "false"
     */
    bool is_str_bool(const std::string& in);

    /**
     * Removes all whitespace from the front of the string
     */
    void strip_leading_whitespace(std::string& in);

    /**
     * Removes all whitespace from the end of the string
     */
    void strip_tailing_whitespace(std::string& in);

    /**
     * Removes whitespace at the given index, recursively, until either pos is out of bounds or pos is not whitespace
     */
    void strip_whitespace_at(std::string& in, std::size_t pos);

    /**
     * Returns true if the given index of the given string is inside any parentheses
     */
    bool is_in_parentheses(const std::string& str, std::size_t i);
}