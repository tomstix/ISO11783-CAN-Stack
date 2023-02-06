//================================================================================================
/// @file isobus_virtual_terminal_objects.cpp
///
/// @brief Implementation of classes that represent the possible items of an object pool
/// @author Tom Stirnkorb
///
/// @copyright 2023 Adrian Del Grosso
//================================================================================================
#include "isobus/isobus/isobus_virtual_terminal_objects.hpp"
#include "isobus/isobus/can_stack_logger.hpp"
#include "isobus/utility/to_string.hpp"
#include "isobus/utility/iop_file_interface.hpp"

/// @brief Converts bytes from a vector at a given iterator to a value of type T
/// @tparam T Type to convert to
/// @param it Iterator of the start of the bytes to convert
/// @param end Iterator of the end of the vector to avoid out of bounds access
/// @returns The converted value
template <typename T>
T convert_bytes_to(std::vector<std::uint8_t>::iterator &it, const std::vector<std::uint8_t>::const_iterator &end)
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
        T tmp = (T)(*it++) << (i * 8);
        value |= tmp;
    }
    return value;
}

namespace isobus
{
    VTObject::objectID_t VTObject::get_object_id() const
    {
        return this->objectID;
    }

    void VTObject::register_update_callback(const VTObjectChangedCallback &callback)
    {
        this->objectChangedCallbacks.push_back(callback);
    }

    void VTObject::call_object_changed_callbacks() const
    {
        for (const auto &callback : this->objectChangedCallbacks)
        {
            callback(this->objectID);
        }
    }

    VTChildObjects const &VTObjectWithChildObjects::get_child_objects() const { return this->childObjects; };

    void VTObjectWithChildObjects::change_child_position(const objectID_t child, const std::uint16_t newX, const std::uint16_t newY)
    {
        auto it = this->childObjects.find(child);
        if (it != this->childObjects.end())
        {
            it->second = std::make_pair(newX, newY);
            call_object_changed_callbacks();
            return;
        }
        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                      "[Object Pool Parser] Failed to change child position: child not found!");
    }

    void VTObjectWithChildObjects::change_child_location(const objectID_t child, const std::uint16_t deltaX, const std::uint16_t deltaY)
    {
        auto it = this->childObjects.find(child);
        if (it != this->childObjects.end())
        {
            it->second.first += deltaX;
            it->second.second += deltaY;
            call_object_changed_callbacks();
            return;
        }
        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                      "[Object Pool Parser] Failed to change child location: child not found!");
    }

    std::vector<std::uint16_t> VTObjectChildMacrosExtension::get_child_macros() const { return this->childMacros; };

    WorkingSetObject::ObjectType WorkingSetObject::get_object_type() const
    {
        return ObjectType::WorkingSet;
    }

    WorkingSetObject::Attribute WorkingSetObject::get_attribute(const Attribute::ID id) const
    {
        if (Attributes::Type == static_cast<Attributes>(id))
        {
            Attribute attr;
            attr.type = Attribute::Type::Uint8;
            attr.value.Uint8 = static_cast<std::uint8_t>(this->get_object_type());
            return attr;
        }
        if (Attributes::BackgroundColour == static_cast<Attributes>(id))
        {
            Attribute attr;
            attr.type = Attribute::Type::Uint8;
            attr.value.Uint8 = this->backgroundColour;
            return attr;
        }
        if (Attributes::Selectable == static_cast<Attributes>(id))
        {
            Attribute attr;
            attr.type = Attribute::Type::Boolean;
            attr.value.Boolean = this->selectable;
            return attr;
        }
        if (Attributes::ActiveMask == static_cast<Attributes>(id))
        {
            Attribute attr;
            attr.type = Attribute::Type::Uint16;
            attr.value.Uint16 = this->activeMask;
            return attr;
        }
        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                      "[Object Pool Parser] Failed to get working set attribute " + to_string(id) + ": attribute not found!");
        return Attribute();
    }

    bool WorkingSetObject::change_attribute(const Attribute::ID, const Attribute &)
    {
        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                      "[Object Pool Parser] Failed to change attribute: working set objects do not have mutable attributes!");
        return false;
    }

    bool WorkingSetObject::parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &it)
    {
        this->objectID = convert_bytes_to<objectID_t>(it, bytes.end());
        it++; // skip object type
        this->backgroundColour = *it++;
        this->selectable = static_cast<bool>(*it++);
        this->activeMask = convert_bytes_to<objectID_t>(it, bytes.end());
        std::uint8_t numObjects = *it++;
        std::uint8_t numMacros = *it++;
        std::uint8_t numLanguages = *it++;

        if (bytes.end() - it < numObjects * 6 + numMacros * 2 + numLanguages * 2)
        {
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to parse working set object: invalid pool size!");
            return false;
        }

        for (int i = 0; i < numObjects; ++i)
        {
            objectID_t id = convert_bytes_to<objectID_t>(it, bytes.end());
            auto x_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            auto y_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            childObjects.emplace(id, std::make_pair(x_location, y_location));
        }

        for (int i = 0; i < numMacros; ++i)
        {
            childMacros.push_back(convert_bytes_to<uint16_t>(it, bytes.end()));
        }

        for (int i = 0; i < numLanguages; ++i)
        {
            char language[] = {static_cast<char>(*it++), static_cast<char>(*it++)};
            childLanguages.emplace_back(language);
        }

        return true;
    }

    std::vector<std::string> WorkingSetObject::get_child_languages() const { return this->childLanguages; };

    void WorkingSetObject::change_active_mask(const objectID_t mask)
    {
        this->activeMask = mask;
        call_object_changed_callbacks();
    }

    void WorkingSetObject::change_background_colour(colour_t colour)
    {
        this->backgroundColour = colour;
        call_object_changed_callbacks();
    }

    DataMaskObject::ObjectType DataMaskObject::get_object_type() const
    {
        return ObjectType::DataMask;
    }

    DataMaskObject::Attribute DataMaskObject::get_attribute(const Attribute::ID id) const
    {
        Attribute attr;
        attr.id = id;
        if (Attributes::Type == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint8;
            attr.value.Uint8 = static_cast<std::uint8_t>(this->get_object_type());
            return attr;
        }
        if (Attributes::BackgroundColour == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint8;
            attr.value.Uint8 = this->backgroundColour;
            return attr;
        }
        if (Attributes::SoftKeyMask == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint16;
            attr.value.Boolean = this->softKeyMask;
            return attr;
        }
        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                      "[Object Pool Parser] Failed to get data mask attribute " + to_string(id) + ": attribute not found!");
        return Attribute();
    }

    bool DataMaskObject::change_attribute(const Attribute::ID id, const Attribute &newAttribute)
    {
        if (Attributes::BackgroundColour == static_cast<Attributes>(id))
        {
            if (Attribute::Type::Uint8 == newAttribute.type)
            {
                this->backgroundColour = newAttribute.value.Uint8;
                call_object_changed_callbacks();
                return true;
            }
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to change data mask attribute background colour: invalid attribute type!");
            return false;
        }
        if (Attributes::SoftKeyMask == static_cast<Attributes>(id))
        {
            if (Attribute::Type::Uint16 == newAttribute.type)
            {
                this->softKeyMask = newAttribute.value.Uint16;
                call_object_changed_callbacks();
                return true;
            }
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to change data mask attribute Soft Key Mask: invalid attribute type!");
            return false;
        }
        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                      "[Object Pool Parser] Failed to change data mask attribute " + to_string(id) + ": attribute not found!");
        return false;
    }

    void DataMaskObject::change_background_colour(colour_t colour)
    {
        this->backgroundColour = colour;
        call_object_changed_callbacks();
    }

    void DataMaskObject::change_soft_key_mask(const objectID_t mask)
    {
        this->softKeyMask = mask;
        call_object_changed_callbacks();
    }

    bool DataMaskObject::parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &it)
    {
        this->objectID = convert_bytes_to<objectID_t>(it, bytes.end());
        it++; // skip object type
        this->backgroundColour = *it++;
        this->softKeyMask = convert_bytes_to<objectID_t>(it, bytes.end());
        std::uint8_t numObjects = *it++;
        std::uint8_t numMacros = *it++;

        if (bytes.end() - it < numObjects * 6 + numMacros * 2)
        {
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to parse data mask object: invalid pool size!");
            return false;
        }

        for (int i = 0; i < numObjects; ++i)
        {
            objectID_t id = convert_bytes_to<objectID_t>(it, bytes.end());
            auto x_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            auto y_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            childObjects.emplace(id, std::make_pair(x_location, y_location));
        }

        for (int i = 0; i < numMacros; ++i)
        {
            childMacros.push_back(convert_bytes_to<uint16_t>(it, bytes.end()));
        }

        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Debug,
                                      "[Object Pool Parser] Successfully parsed data mask object!");

        return true;
    }

    AlarmMaskObject::ObjectType AlarmMaskObject::get_object_type() const
    {
        return ObjectType::AlarmMask;
    }

    AlarmMaskObject::Attribute AlarmMaskObject::get_attribute(const Attribute::ID id) const
    {
        if (DataMaskObject::get_attribute(id).id != Attribute::NULL_AID)
        {
            return DataMaskObject::get_attribute(id);
        }
        Attribute attr;
        attr.id = id;
        if (Attributes::Priority == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint8;
            attr.value.Uint8 = this->priority;
            return attr;
        }
        if (Attributes::AcousticSignal == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint8;
            attr.value.Uint8 = this->acousticSignal;
            return attr;
        }
        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                      "[Object Pool Parser] Failed to get data mask attribute " + to_string(id) + ": attribute not found!");
        return Attribute();
    }

    bool AlarmMaskObject::change_attribute(const Attribute::ID id, const Attribute &newAttribute)
    {
        if (true == DataMaskObject::change_attribute(id, newAttribute))
        {
            return true;
        }
        if (Attributes::Priority == static_cast<Attributes>(id))
        {
            if (Attribute::Type::Uint8 == newAttribute.type)
            {
                this->priority = newAttribute.value.Uint8;
                call_object_changed_callbacks();
                return true;
            }
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to change data mask attribute priority: invalid attribute type!");
            return false;
        }
        return false;
    }

    bool AlarmMaskObject::parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &it)
    {
        this->objectID = convert_bytes_to<objectID_t>(it, bytes.end());
        it++; // skip object type
        this->backgroundColour = *it++;
        this->softKeyMask = convert_bytes_to<objectID_t>(it, bytes.end());
        this->priority = *it++;
        this->acousticSignal = *it++;
        std::uint8_t numObjects = *it++;
        std::uint8_t numMacros = *it++;

        if (bytes.end() - it < numObjects * 6 + numMacros * 2)
        {
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to parse data mask object: invalid pool size!");
            return false;
        }

        for (int i = 0; i < numObjects; ++i)
        {
            objectID_t id = convert_bytes_to<objectID_t>(it, bytes.end());
            auto x_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            auto y_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            childObjects.emplace(id, std::make_pair(x_location, y_location));
        }

        for (int i = 0; i < numMacros; ++i)
        {
            childMacros.push_back(convert_bytes_to<uint16_t>(it, bytes.end()));
        }

        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Debug,
                                      "[Object Pool Parser] Successfully parsed data mask object!");

        return true;
    }

    ContainerObject::ObjectType isobus::ContainerObject::get_object_type() const
    {
        return ObjectType::Container;
    }

    ContainerObject::Attribute ContainerObject::get_attribute(const Attribute::ID id) const
    {
        Attribute attr;
        attr.id = id;
        if (Attributes::Type == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint8;
            attr.value.Uint8 = static_cast<std::uint8_t>(this->get_object_type());
            return attr;
        }
        if (Attributes::Width == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint16;
            attr.value.Uint16 = this->width;
            return attr;
        }
        if (Attributes::Height == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint16;
            attr.value.Uint16 = this->height;
            return attr;
        }
        if (Attributes::Hidden == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Boolean;
            attr.value.Boolean = this->hidden;
            return attr;
        }
        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                      "[Object Pool Parser] Failed to get data mask attribute " + to_string(id) + ": attribute not found!");
        return Attribute();
    }

    bool ContainerObject::change_attribute(const Attribute::ID id, const Attribute &newAttribute)
    {
        if (Attributes::Width == static_cast<Attributes>(id))
        {
            if (Attribute::Type::Uint16 == newAttribute.type)
            {
                this->width = newAttribute.value.Uint16;
                call_object_changed_callbacks();
                return true;
            }
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to change data mask attribute width: invalid attribute type!");
            return false;
        }
        if (Attributes::Height == static_cast<Attributes>(id))
        {
            if (Attribute::Type::Uint16 == newAttribute.type)
            {
                this->height = newAttribute.value.Uint16;
                call_object_changed_callbacks();
                return true;
            }
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to change data mask attribute height: invalid attribute type!");
            return false;
        }
        if (Attributes::Hidden == static_cast<Attributes>(id))
        {
            if (Attribute::Type::Boolean == newAttribute.type)
            {
                this->hidden = newAttribute.value.Boolean;
                call_object_changed_callbacks();
                return true;
            }
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to change data mask attribute hidden: invalid attribute type!");
            return false;
        }
        return false;
    }

    bool ContainerObject::change_size(const uint16_t newWidth, const uint16_t newHeight)
    {
        this->width = newWidth;
        this->height = newHeight;
        call_object_changed_callbacks();
        return true;
    }

    bool ContainerObject::parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &it)
    {
        this->objectID = convert_bytes_to<objectID_t>(it, bytes.end());
        *it++; // skip object type
        this->width = convert_bytes_to<uint16_t>(it, bytes.end());
        this->height = convert_bytes_to<uint16_t>(it, bytes.end());
        this->hidden = *it++ != 0;
        auto numObjects = convert_bytes_to<uint8_t>(it, bytes.end());
        auto numMacros = convert_bytes_to<uint8_t>(it, bytes.end());
        if (bytes.end() - it < numObjects * 6 + numMacros * 2)
        {
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to parse data mask object: invalid pool size!");
            return false;
        }
        for (auto i = 0; i < numObjects; ++i)
        {
            auto id = convert_bytes_to<objectID_t>(it, bytes.end());
            auto x_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            auto y_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            childObjects.emplace(id, std::make_pair(x_location, y_location));
        }
        for (auto i = 0; i < numMacros; ++i)
        {
            childMacros.push_back(convert_bytes_to<uint16_t>(it, bytes.end()));
        }
        return true;
    }

    uint16_t ContainerObject::get_width() const
    {
        return this->width;
    }

    uint16_t ContainerObject::get_height() const
    {
        return this->height;
    }

    SoftKeyMaskObject::ObjectType SoftKeyMaskObject::get_object_type() const
    {
        return ObjectType::SoftKeyMask;
    }

    SoftKeyMaskObject::Attribute SoftKeyMaskObject::get_attribute(const Attribute::ID id) const
    {
        Attribute attr;
        if (Attributes::Type == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint8;
            attr.value.Uint8 = static_cast<std::uint8_t>(this->get_object_type());
            return attr;
        }
        if (Attributes::BackgroundColour == static_cast<Attributes>(id))
        {
            attr.type = Attribute::Type::Uint8;
            attr.value.Uint8 = this->backgroundColour;
            return attr;
        }
        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                      "[Object Pool Parser] Failed to get soft key mask attribute "
                                      + to_string(id) + ": attribute not found!");
        return Attribute();
    }

    bool SoftKeyMaskObject::change_attribute(const Attribute::ID id, const Attribute &newAttribute)
    {
        if (Attributes::BackgroundColour == static_cast<Attributes>(id))
        {
            if (Attribute::Type::Uint8 == newAttribute.type)
            {
                this->backgroundColour = newAttribute.value.Uint8;
                call_object_changed_callbacks();
                return true;
            }
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to change soft key mask attribute background colour: invalid attribute type!");
            return false;
        }
        return false;
    }

    bool SoftKeyMaskObject::change_background_colour(const std::uint8_t newColour)
    {
        this->backgroundColour = newColour;
        call_object_changed_callbacks();
        return true;
    }

    std::vector<SoftKeyMaskObject::objectID_t> SoftKeyMaskObject::get_child_objects() const
    {
        return this->childObjects;
    }

    bool SoftKeyMaskObject::parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &it)
    {
        this->objectID = convert_bytes_to<objectID_t>(it, bytes.end());
        *it++; // skip object type
        this->backgroundColour = *it++;
        auto numObjects = convert_bytes_to<uint8_t>(it, bytes.end());
        auto numMacros = convert_bytes_to<uint8_t>(it, bytes.end());
        if (bytes.end() - it < numObjects * sizeof(objectID_t) + numMacros * sizeof(objectID_t))
        {
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to parse soft key mask object: invalid pool size!");
            return false;
        }
        for (auto i = 0; i < numObjects; ++i)
        {
            auto id = convert_bytes_to<objectID_t>(it, bytes.end());
            childObjects.push_back(id);
        }
        for (auto i = 0; i < numMacros; ++i)
        {
            childMacros.push_back(convert_bytes_to<uint16_t>(it, bytes.end()));
        }
        return false;
    }

    bool ObjectPool::parse(std::vector<std::uint8_t> &binaryPool)
    {
        auto iterator = binaryPool.begin();
        while (iterator != binaryPool.end())
        {
            auto objecttype = static_cast<ObjectType>(iterator[2]);
            switch (objecttype)
            {
            case ObjectType::WorkingSet:
            {
                auto workingsetObject = std::make_shared<WorkingSetObject>(this);
                if (!workingsetObject->parse(binaryPool, iterator))
                    return false;
                objects[workingsetObject->get_object_id()] = static_cast<std::shared_ptr<VTObject>>(workingsetObject);
                break;
            }
            case ObjectType::DataMask:
            {
                auto datamaskObject = std::make_shared<DataMaskObject>(this);
                if (!datamaskObject->parse(binaryPool, iterator))
                    return false;
                objects[datamaskObject->get_object_id()] = static_cast<std::shared_ptr<VTObject>>(datamaskObject);
                break;
            }
            default:
            {
                CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                              "[Object Pool Parser] Invalid or unsupported object pool element detected: " + isobus::to_string(static_cast<uint8_t>(objecttype)));
                return false;
            }
            }
        }
        this->versionHash = IOPFileInterface::hash_object_pool_to_version(binaryPool);
        return true;
    }

    std::shared_ptr<VTObject> ObjectPool::get_object(objectID_t objectID) const
    {
        auto it = objects.find(objectID);
        if (it == objects.end())
        {
            CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Error,
                                          "[Object Pool Parser] Failed to get object: object not found!");
            return nullptr;
        }
        return it->second;
    }
} // namespace isobus