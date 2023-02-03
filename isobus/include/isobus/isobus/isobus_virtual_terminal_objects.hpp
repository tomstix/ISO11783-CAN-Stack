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
#include <functional>

namespace isobus
{
    // Forward declaration of Object Pool and VirtualTerminalObject to use as pointer
    class ObjectPool;
    class VirtualTerminalObject;

    /// @brief defintion of a callback when an object is changed
    /// @param objectID The ID of the object that changed
    using VTObjectChangedCallback = std::function<void(const VirtualTerminalBase::ObjectID objectID)>;

    using VTObjectMap = std::map<VirtualTerminalBase::ObjectID, std::shared_ptr<VirtualTerminalObject>>;

    using VTObjectCoordinates = std::pair<std::uint16_t, std::uint16_t>;

    /// @brief Typedef for a child object consisting of id, x position, y position
    using VTChildObjects = std::map<VirtualTerminalBase::ObjectID, VTObjectCoordinates>;

    using VTChildMacros = std::vector<std::uint16_t>;

    using VTChildLanguages = std::vector<std::string>;

    class VirtualTerminalObject : public VirtualTerminalBase
    {
    public:
        VirtualTerminalObject() = delete;
        VirtualTerminalObject(ObjectPool *parentObjectPool) : parentObjectPool(parentObjectPool) {}
        virtual ~VirtualTerminalObject() = default;

        /// @brief Get the ID of a VT Object
        /// @returns the object ID
        ObjectID get_object_id() const;

        /// @brief Get the type of a VT Object
        /// @returns the object type
        virtual ObjectType get_object_type() const = 0;

        /// @brief Register a callback that is called when the object is changed
        /// @param callback The callback to register
        void register_update_callback(const VTObjectChangedCallback &callback);

    protected:
        /// @brief ID of the VT Object
        ObjectID objectID = 0;

        /// @brief Pointer to the parent object pool
        ObjectPool *parentObjectPool = nullptr;

        /// @brief A vector of callbacks that are called when the object is changed
        std::vector<VTObjectChangedCallback> objectChangedCallbacks;

        /// @brief Call all registered callbacks
        void call_object_changed_callbacks() const;
    };

    /// @brief A class that represents a Working Set Object
    class WorkingSetObject : public VirtualTerminalObject
    {
    public:
        using VirtualTerminalObject::VirtualTerminalObject;

        /// @brief Get the object type
        /// @returns ObjectType
        ObjectType get_object_type() const final;

        /// @brief Parse a WorkingSetObject from a byte vector
        /// @param iterator An iterator pointing to the start of the object
        /// @param bytes The object pool byte vector
        /// @returns True if successful
        bool parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &begin);

        /// @brief Get the child objects of the working set
        /// @returns The VTChildObjects
        VTChildObjects const &get_child_objects() const;

        /// @brief Get the child macros of the working set
        /// @returns A vector of uint16_t values representing the macros
        VTChildMacros get_child_macros() const;

        /// @brief Get the languages contained in the working set
        /// @returns A string vector of the languages
        VTChildLanguages get_child_languages() const;

        /// @brief Change the active mask of the working set
        /// @param mask Object ID of the mask to activate
        void change_active_mask(const ObjectID mask);

        /// @brief Change the background color of the working set
        /// @param color The new background color
        void change_background_color(std::uint8_t color);

        /// @brief Change the position of a child object in absolute coordinates
        /// @param child The ID of the child object
        /// @param newX The new x position
        /// @param newY The new y position
        void change_child_position(const ObjectID child, const std::uint16_t newX, const std::uint16_t newY);

        /// @brief Change the position of a child object in relative coordinates
        /// @param child The ID of the child object
        /// @param deltaX The change in x position
        /// @param deltaY The change in y position
        void change_child_location(const ObjectID child, const std::uint16_t deltaX, const std::uint16_t deltaY);

    private:
        std::uint8_t backgroundColor = 0;
        bool selectable = false;
        ObjectID activeMask = 0;
        VTChildObjects childObjects;
        VTChildMacros childMacros;
        VTChildLanguages childLanguages;
    };

    /// @brief A class that represents a Data Mask object
    class DataMaskObject : public VirtualTerminalObject
    {
    public:
        using VirtualTerminalObject::VirtualTerminalObject;
        /// @brief Get the object type
        /// @returns ObjectType
        ObjectType get_object_type() const final;

        /// @brief Parse a DataMaskObject from a byte vector
        /// @param begin An iterator pointing to the start of the object
        /// @param bytes The object pool byte vector
        /// @returns True if successful
        bool parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &begin);

    private:
        uint8_t backgroundColor;
        ObjectID softKeyMask;
        VTChildObjects childObjects;
        VTChildMacros childMacros;
    };

    /// @brief A class that contains an object pool
    class ObjectPool : public VirtualTerminalBase
    {
    public:
        /// @brief Parses an object pool
        /// @param binaryPool The binary representation of the pool
        /// @returns true if parsing succeeded
        bool parse(std::vector<std::uint8_t> &binaryPool);

        std::shared_ptr<VirtualTerminalObject> get_object(const ObjectID objectID) const;

    private:
        VTObjectMap objects;
        std::string versionHash;
    };
} // namespace isobus

#endif // ISOBUS_VIRTUAL_TERMINAL_OBJECTS_HPP
