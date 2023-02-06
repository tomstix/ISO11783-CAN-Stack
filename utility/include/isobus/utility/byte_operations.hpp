//================================================================================================
/// @file byte_operations.hpp
///
/// @brief Utility functions for byte operations
/// @author Tom Stirnkorb
///
/// @copyright 2022 Adrian Del Grosso
//================================================================================================

#ifndef BYTE_OPERATIONS_HPP
#define BYTE_OPERATIONS_HPP

#include <string>
#include <vector>

namespace isobus
{
    enum class ByteOrder
    {
        LittleEndian,
        BigEndian
    };

    /// @brief Converts bytes from a vector at a given iterator to a value of type T
    /// @tparam T Type to convert to
    /// @param it Iterator of the start of the bytes to convert
    /// @param end Iterator of the end of the vector to avoid out of bounds access
    /// @returns The converted value
    template <typename T>
    T convert_byte_vector_to(std::vector<std::uint8_t>::iterator &it, const std::vector<std::uint8_t>::const_iterator &end, ByteOrder order = ByteOrder::LittleEndian)
    {
        T value = *it++;
        for (std::uint8_t i = 1; i < static_cast<std::uint8_t>(sizeof(T)); ++i)
        {
            if (it == end)
            {
                isobus::CANStackLogger::CAN_stack_log(isobus::CANStackLogger::LoggingLevel::Error,
                                                      "[Object Pool Parser] Reached unexpected end of vector while parsing bytes!");
                return value;
            }
            if (order == ByteOrder::LittleEndian)
            {
                T tmp = (T)(*it++) << (i * 8);
                value |= tmp;
            }
            else
            {
                T tmp = (T)(*it++) << ((sizeof(T) - i) * 8);
                value |= tmp;
            }
        }
        return value;
    }
}

#endif // BYTE_OPERATIONS_HPP