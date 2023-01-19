//================================================================================================
/// @file isobus_virtual_terminal_objects.cpp
///
/// @brief Implementation of classes that represent the possible items of an object pool
/// @author Tom Stirnkorb
///
/// @copyright 2023 Adrian Del Grosso
//================================================================================================
#include "isobus/isobus/isobus_virtual_terminal_objects.hpp"
#include "isobus/isobus/can_warning_logger.hpp"
#include "isobus/utility/to_string.hpp"

#include <iostream>

namespace isobus
{
    VirtualTerminalObject::ObjectType VirtualTerminalObject::get_object_type() const
    {
        return ObjectType::Reserved;
    }

    VirtualTerminalObject::ObjectID VirtualTerminalObject::get_object_id() const
    {
        return this->objectID;
    }

    WorkingSetObject::ObjectType WorkingSetObject::get_object_type() const
    {
        return ObjectType::WorkingSet;
    }

    bool WorkingSetObject::parse(const std::vector<std::uint8_t> &bytes, std::vector<std::uint8_t>::iterator &begin)
    {
        std::vector<std::uint8_t>::iterator it = begin;

        this->objectID = static_cast<ObjectID>(*it | *++it << 8);
        ++it; // skip object type
        this->backgroundColor = *++it;
        this->selectable = static_cast<bool>(*++it);
        this->activeMask = static_cast<ObjectID>(*++it | *++it << 8);
        std::uint8_t numObjects = *++it;
        std::uint8_t numMacros = *++it;
        std::uint8_t numLanguages = *++it;

        if (bytes.end() - it < numObjects * 6 + numMacros * 2 + numLanguages * 2)
        {
            CANStackLogger::CAN_stack_log("[Object Pool Parser] Failed to parse working set object: invalid pool size!");
            return false;
        }

        for (int i = 0; i < numObjects; ++i)
        {
            ObjectID id = *++it | *++it << 8;
            std::int16_t x_location = *++it | *++it << 8;
            std::int16_t y_location = *++it | *++it << 8;
            childObjects.push_back(std::make_tuple(id, x_location, y_location));
        }

        for (int i = 0; i < numMacros; ++i)
        {
            childMacros.push_back(static_cast<std::int16_t>(*++it | *++it << 8));
        }

        for (int i = 0; i < numLanguages; ++i)
        {
            char language[] = {static_cast<char>(*++it), static_cast<char>(*++it)};
            childLanguages.push_back(std::string(language));
        }

        return true;
    }

    std::vector<WorkingSetObject::ChildObject> WorkingSetObject::get_child_objects() const { return this->childObjects; };

    std::vector<std::uint16_t> WorkingSetObject::get_child_macros() const { return this->childMacros; };

    std::vector<std::string> WorkingSetObject::get_child_languages() const { return this->childLanguages; };

    bool ObjectPool::parse(std::vector<std::uint8_t> &binaryPool)
    {
        bool validPool = true;
        std::vector<std::uint8_t>::iterator it = binaryPool.begin();
        while (it < binaryPool.end() && validPool)
        {
            ObjectID id = *it | *it++ << 8;
            ObjectType type = static_cast<ObjectType>(*it++);
            switch (type)
            {
            case ObjectType::WorkingSet:
            {
                auto workingsetObject = std::make_shared<WorkingSetObject>();
                auto size = workingsetObject->parse(binaryPool, it);
                objects[id] = static_cast<std::shared_ptr<VirtualTerminalObject>>(workingsetObject);
                it = it + size;
                break;
            }
            default:
            {
                CANStackLogger::CAN_stack_log("[Object Pool Parser] Invalid or unsupported object pool element detected: " + isobus::to_string(static_cast<uint8_t>(type)));
                validPool = false;
                break;
            }
            }
        }
        return false;
    }
}
