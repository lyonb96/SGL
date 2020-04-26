#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <type_traits>

/**
 * Reads the requested type from the buffer, making sure it's aligned properly
 * Also can perform endian swapping if SGL_BIG_ENDIAN is defined
 */
template <class T>
T read_from_buffer(std::uint8_t* buffer)
{
	T ret = 0;
	std::memcpy(&ret, buffer, sizeof(T));

#ifdef SGL_BIG_ENDIAN
	// do endian swap on integral types
	if constexpr (std::is_integral_v<T>)
	{
		constexpr const std::size_t Tsize = sizeof(T);
		if constexpr (Tsize == 2)
		{
			_byteswap_ushort(ret);
		}
		if constexpr (Tsize == 4)
		{
			_byteswap_ulong(ret);
		}
		if constexpr (Tsize == 8)
		{
			_byteswap_uint64(ret);
		}
	}
#endif

	return ret;
}

/**
 * Stores the requested value into the given buffer
 * Also can perform endian swapping if SGL_BIG_ENDIAN is defined
 */
template <class T>
void store_to_buffer(std::uint8_t* buffer, std::size_t max_size, const T& value)
{
	T write = value;

#ifdef SGL_BIG_ENDIAN
	// do endian swap on integral types
	if constexpr (std::is_integral_v<T>)
	{
		constexpr const std::size_t Tsize = sizeof(T);
		if constexpr (Tsize == 2)
		{
			_byteswap_ushort(write);
		}
		if constexpr (Tsize == 4)
		{
			_byteswap_ulong(write);
		}
		if constexpr (Tsize == 8)
		{
			_byteswap_uint64(write);
		}
	}
#endif

	// only write up to the size of the buffer
	std::memcpy(buffer, &write, std::min(max_size, sizeof(T)));
}