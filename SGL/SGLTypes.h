#pragma once

#include <iostream>
#include <unordered_map>
#include <string>

// SGL type definitions and registration

/**
 * Holds information pertaining to an SGL type
 */
struct SGLType
{
	// Name of the type as it is written in SGL
	std::string TypeName = "";
	// Size of the type in bytes
	int TypeSize = 0;
	// Byte alignment required by the type
	int TypeAlignment = 0;
};

// Map of registered types and their specifiers
inline std::unordered_map<std::string, SGLType>& get_types()
{
	static std::unordered_map<std::string, SGLType> map;
	return map;
}

/**
 * Returns true if a type with the given name is registered
 */
inline bool is_type_registered(const std::string& specifier)
{
	if (get_types().count(specifier) > 0)
	{
		return true;
	}
	return false;
}

/**
 * Registers a new SGL type with the given specifier
 */
template <typename T>
void register_type(const std::string& specifier)
{
	// Check if type is already registered
	if (is_type_registered(specifier))
	{
		std::cout << "Type with specifier " << specifier << " already registered." << std::endl;
		return;
	}

	// Create the new type
	SGLType type;
	type.TypeName = specifier;
	type.TypeSize = sizeof(T);
	type.TypeAlignment = alignof(T);
	// Add it to the type map
	get_types()[specifier] = type;
}

/**
 * Registers SGL's PODs:
 * int32 - 32-bit signed int
 * float - 32-bit float
 * bool  - true/false
 * void  - typeless expression (mainly used internally)
 */
void register_datatypes();

// Helper macro to register the type with a name exactly matching the C++ type's name
#define SGL_REGISTER_TYPE(TYPE) register_type<TYPE>(#TYPE);