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
        T tmp = (*it++) << (i * 8);
        value |= tmp;
    }
    return value;
}

namespace isobus
{
    VirtualTerminalObject::ObjectID VirtualTerminalObject::get_object_id() const
    {
        return this->objectID;
    }

    void VirtualTerminalObject::register_update_callback(const VTObjectChangedCallback &callback)
    {
        this->objectChangedCallbacks.push_back(callback);
    }

    void VirtualTerminalObject::call_object_changed_callbacks() const
    {
        for (const auto &callback : this->objectChangedCallbacks)
        {
            callback(this->objectID);
        }
    }

    WorkingSetObject::ObjectType WorkingSetObject::get_object_type() const
    {
        return ObjectType::WorkingSet;
    }

    bool WorkingSetObject::parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &it)
    {
        this->objectID = convert_bytes_to<ObjectID>(it, bytes.end());
        it++; // skip object type
        this->backgroundColor = *it++;
        this->selectable = static_cast<bool>(*it++);
        this->activeMask = convert_bytes_to<ObjectID>(it, bytes.end());
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
            ObjectID id = convert_bytes_to<ObjectID>(it, bytes.end());
            auto x_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            auto y_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            childObjects.emplace(id, std::make_pair(x_location, y_location));
        }

        for (int i = 0; i < numMacros; ++i)
        {
            childMacros.push_back(static_cast<std::int16_t>(*it++ | *it++ << 8));
        }

        for (int i = 0; i < numLanguages; ++i)
        {
            char language[] = {static_cast<char>(*it++), static_cast<char>(*it++)};
            childLanguages.emplace_back(language);
        }

        return true;
    }

    VTChildObjects const &WorkingSetObject::get_child_objects() const { return this->childObjects; };

    std::vector<std::uint16_t> WorkingSetObject::get_child_macros() const { return this->childMacros; };

    std::vector<std::string> WorkingSetObject::get_child_languages() const { return this->childLanguages; };

    void WorkingSetObject::change_active_mask(const ObjectID mask)
    {
        this->activeMask = mask;
        call_object_changed_callbacks();
    }

    void WorkingSetObject::change_background_color(std::uint8_t color)
    {
        this->backgroundColor = color;
        call_object_changed_callbacks();
    }

    void WorkingSetObject::change_child_position(const ObjectID child, const std::uint16_t newX, const std::uint16_t newY)
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

    void WorkingSetObject::change_child_location(const ObjectID child, const std::uint16_t deltaX, const std::uint16_t deltaY)
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

    DataMaskObject::ObjectType DataMaskObject::get_object_type() const
    {
        return ObjectType::DataMask;
    }

    bool DataMaskObject::parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &it)
    {
        this->objectID = convert_bytes_to<ObjectID>(it, bytes.end());
        it++; // skip object type
        this->backgroundColor = *it++;
        this->softKeyMask = convert_bytes_to<ObjectID>(it, bytes.end());
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
            ObjectID id = convert_bytes_to<ObjectID>(it, bytes.end());
            auto x_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            auto y_location = convert_bytes_to<std::int16_t>(it, bytes.end());
            childObjects.emplace(id, std::make_pair(x_location, y_location));
        }

        for (int i = 0; i < numMacros; ++i)
        {
            childMacros.push_back(static_cast<std::int16_t>(*it++ | *it++ << 8));
        }

        CANStackLogger::CAN_stack_log(CANStackLogger::LoggingLevel::Debug,
                                      "[Object Pool Parser] Successfully parsed data mask object!");

        return true;
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
                objects[workingsetObject->get_object_id()] = static_cast<std::shared_ptr<VirtualTerminalObject>>(workingsetObject);
                break;
            }
            case ObjectType::DataMask:
            {
                auto datamaskObject = std::make_shared<DataMaskObject>(this);
                if (!datamaskObject->parse(binaryPool, iterator))
                    return false;
                objects[datamaskObject->get_object_id()] = static_cast<std::shared_ptr<VirtualTerminalObject>>(datamaskObject);
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

    std::shared_ptr<VirtualTerminalObject> ObjectPool::get_object(ObjectID objectID) const
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