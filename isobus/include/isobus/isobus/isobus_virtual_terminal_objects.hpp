//================================================================================================
/// @file isobus_virtual_terminal_objects.hpp
///
/// @brief A collection of classes that represent the possible items of an object pool
/// @author Tom Stirnkorb
///
/// @copyright 2023 Adrian Del Grosso
//================================================================================================

#ifndef ISOBUS_VIRTUAL_TERMINAL_OBJECTS_HPP
#define ISOBUS_VIRTUAL_TERMINAL_OBJECTS_HPP

#include "isobus/isobus/isobus_virtual_terminal_base.hpp"

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <string>

namespace isobus
{
    class VirtualTerminalObject : public VirtualTerminalBase
    {
    public:
        /// @brief Get the ID of a VT Object
        /// @returns the object ID
        ObjectID get_object_id() const;

        /// @brief Get the type of a VT Object
        /// @returns the object type
        virtual ObjectType get_object_type() const;

    protected:
        ObjectID objectID = 0;
    };

    /// @brief A class that represents a Working Set Object
    class WorkingSetObject : public VirtualTerminalObject
    {
    public:
        /// @brief Get the object type
        /// @returns ObjectType
        ObjectType get_object_type() const override;

        /// @brief Parse a WorkingSetObject from a byte vector
        /// @param iterator An iterator pointing to the start of the object
        /// @param bytes The object pool byte vector
        /// @returns True if successful
        bool parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &begin);

        /// @brief Typedef for a child object consisting if id, x position, y position
        typedef std::tuple<ObjectID, std::uint16_t, std::uint16_t> ChildObject;

        /// @brief Get the child objects of the working set
        /// @returns A vector of ChildObjects
        std::vector<ChildObject> get_child_objects() const;

        /// @brief Get the child macros of the working set
        /// @returns A vector of uint16_t values representing the macros
        std::vector<std::uint16_t> get_child_macros() const;

        /// @brief Get the languages contained in the working set
        /// @returns A string vector of the languages
        std::vector<std::string> get_child_languages() const;

    private:
        std::uint8_t backgroundColor = 0;
        bool selectable = false;
        ObjectID activeMask = 0;
        std::vector<ChildObject> childObjects;
        std::vector<std::uint16_t> childMacros;
        std::vector<std::string> childLanguages;
    };

    /// @brief A class that represents a Data Mask object
    class DataMaskObject : public VirtualTerminalObject
    {
    public:
        bool parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &begin);
    };

    /// @brief A class that contains an object pool
    class ObjectPool : public VirtualTerminalBase
    {
    public:
        /// @brief Parses an object pool
        /// @param binaryPool The binary representation of the pool
        /// @returns true if parsing succeeded
        bool parse(std::vector<std::uint8_t> &binaryPool);
        std::map<ObjectID, std::shared_ptr<VirtualTerminalObject>> &get_objects() const;

    private:
        std::map<ObjectID, std::shared_ptr<VirtualTerminalObject>> objects;
    };
} // namespace isobus

#endif // ISOBUS_VIRTUAL_TERMINAL_OBJECTS_HPP
