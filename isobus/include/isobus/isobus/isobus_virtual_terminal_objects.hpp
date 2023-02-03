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
    // Forward declaration of Object Pool and VTObject to use as pointer
    class ObjectPool;
    class VTObject;

    /// @brief defintion of a callback when an object is changed
    /// @param objectID The ID of the object that changed
    using VTObjectChangedCallback = std::function<void(const VirtualTerminalBase::ObjectID objectID)>;

    using VTObjectMap = std::map<VirtualTerminalBase::ObjectID, std::shared_ptr<VTObject>>;

    using VTObjectCoordinates = std::pair<std::uint16_t, std::uint16_t>;

    /// @brief Typedef for a child object consisting of id, x position, y position
    using VTChildObjects = std::map<VirtualTerminalBase::ObjectID, VTObjectCoordinates>;

    using VTChildMacros = std::vector<std::uint16_t>;

    using VTChildLanguages = std::vector<std::string>;

    class VTObject : public VirtualTerminalBase
    {
    public:
        VTObject() = delete;
        VTObject(ObjectPool *parentObjectPool) : parentObjectPool(parentObjectPool) {}
        virtual ~VTObject() = default;

        /// @brief Get the ID of a VT Object
        /// @returns the object ID
        ObjectID get_object_id() const;

        /// @brief Get the type of a VT Object
        /// @returns the object type
        virtual ObjectType get_object_type() const = 0;

        /// @brief Get the current value of an objects attribute
        /// @param id The attribute's ID
        /// @param attribute The attribute to write to
        /// @return The AttributeType that the Attribute is using
        virtual Attribute get_attribute(const Attribute::ID id) const = 0;

        /// @brief Change an attribute of the object
        /// @param id The attribute to change
        /// @param newAttribute The new value for the attribute
        /// @return True if successful
        virtual bool change_attribute(const Attribute::ID id, const Attribute &newAttribute) = 0;

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

    class VTObjectWithChildObjects : public VTObject
    {
    public:
        using VTObject::VTObject;

        /// @brief Get the child objects of the object
        /// @returns The VTChildObjects
        VTChildObjects const &get_child_objects() const;

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

    protected:
        VTChildObjects childObjects;
    };

    class VTObjectChildMacrosExtension : public VirtualTerminalBase
    {
    public:
        /// @brief Get the child macros of the working set
        /// @returns A vector of uint16_t values representing the macros
        VTChildMacros get_child_macros() const;

    protected:
        VTChildMacros childMacros;
    };

    /// @brief A class that represents a Working Set Object
    class WorkingSetObject : public VTObjectWithChildObjects, public VTObjectChildMacrosExtension
    {
    public:
        using VTObjectWithChildObjects::VTObjectWithChildObjects;

        /// @brief Get the object type
        /// @returns ObjectType
        ObjectType get_object_type() const final;

        /// @brief Get the current value of an objects attribute
        /// @param id The attribute's ID
        /// @param attribute The attribute to write to
        /// @return The AttributeType that the Attribute is using
        Attribute get_attribute(const Attribute::ID id) const final;

        /// @brief Change an attribute of the object
        /// @param id The attribute to change
        /// @param newAttribute The new value for the attribute
        /// @return True if successful
        bool change_attribute(const Attribute::ID id, const Attribute &newAttribute) final;

        /// @brief Parse a WorkingSetObject from a byte vector
        /// @param iterator An iterator pointing to the start of the object
        /// @param bytes The object pool byte vector
        /// @returns True if successful
        bool parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &begin);

        /// @brief Get the languages contained in the working set
        /// @returns A string vector of the languages
        VTChildLanguages get_child_languages() const;

        /// @brief Change the active mask of the working set
        /// @param mask Object ID of the mask to activate
        void change_active_mask(const ObjectID mask);

        /// @brief Change the background color of the working set
        /// @param color The new background color
        void change_background_color(std::uint8_t color);

    private:
        std::uint8_t backgroundColor = 0;
        bool selectable = false;
        ObjectID activeMask = 0;
        VTChildLanguages childLanguages;
        enum class Attributes : Attribute::ID
        {
            Type = 0x00,
            BackgroundColor = 0x01,
            Selectable = 0x02,
            ActiveMask = 0x03
        };
    };

    /// @brief A class that represents a Data Mask object
    class DataMaskObject : public VTObjectWithChildObjects, public VTObjectChildMacrosExtension
    {
    public:
        using VTObjectWithChildObjects::VTObjectWithChildObjects;

        /// @brief Get the object type
        /// @returns ObjectType
        ObjectType get_object_type() const final;

        /// @brief Get the current value of an objects attribute
        /// @param id The attribute's ID
        /// @param attribute The attribute to write to
        /// @return The AttributeType that the Attribute is using
        Attribute get_attribute(const Attribute::ID id) const final;

        /// @brief Change an attribute of the object
        /// @param id The attribute to change
        /// @param newAttribute The new value for the attribute
        /// @return True if successful
        bool change_attribute(const Attribute::ID id, const Attribute &newAttribute) final;

        /// @brief Change the background color of the data mask
        /// @param color The new background color
        void change_background_color(std::uint8_t color);

        /// @brief Change the soft key mask of the data mask
        /// @param mask ObjectID of the new soft key mask
        void change_soft_key_mask(const ObjectID mask);

        /// @brief Parse a DataMaskObject from a byte vector
        /// @param begin An iterator pointing to the start of the object
        /// @param bytes The object pool byte vector
        /// @returns True if successful
        bool parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &begin);

    private:
        uint8_t backgroundColor;
        ObjectID softKeyMask;
        enum class Attributes : Attribute::ID
        {
            Type = 0,
            BackgroundColor = 1,
            SoftKeyMask = 2
        };
    };

    /// @brief A class that contains an object pool
    class ObjectPool : public VirtualTerminalBase
    {
    public:
        /// @brief Parses an object pool
        /// @param binaryPool The binary representation of the pool
        /// @returns true if parsing succeeded
        bool parse(std::vector<std::uint8_t> &binaryPool);

        std::shared_ptr<VTObject> get_object(const ObjectID objectID) const;

    private:
        VTObjectMap objects;
        std::string versionHash;
    };
} // namespace isobus

#endif // ISOBUS_VIRTUAL_TERMINAL_OBJECTS_HPP
