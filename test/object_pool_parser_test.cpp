#include <gtest/gtest.h>

#include <vector>
#include <memory>
#include <iostream>

#include "isobus/isobus/isobus_virtual_terminal_objects.hpp"
#include "isobus/isobus/can_stack_logger.hpp"

class CustomLogger : public isobus::CANStackLogger
{
public:
    void sink_CAN_stack_log(CANStackLogger::LoggingLevel level, const std::string &text) override
    {
        switch (level)
        {
        case LoggingLevel::Debug:
        {
            std::cout << "["
                      << "\033[1;36m"
                      << "Debug"
                      << "\033[0m"
                      << "]";
        }
        break;

        case LoggingLevel::Info:
        {
            std::cout << "["
                      << "\033[1;32m"
                      << "Info"
                      << "\033[0m"
                      << "]";
        }
        break;

        case LoggingLevel::Warning:
        {
            std::cout << "["
                      << "\033[1;33m"
                      << "Warn"
                      << "\033[0m"
                      << "]";
        }
        break;

        case LoggingLevel::Error:
        {
            std::cout << "["
                      << "\033[1;31m"
                      << "Error"
                      << "\033[0m"
                      << "]";
        }
        break;

        case LoggingLevel::Critical:
        {
            std::cout << "["
                      << "\033[1;35m"
                      << "Critical"
                      << "\033[0m"
                      << "]";
        }
        break;
        }
        std::cout << text << std::endl; // Write the text to stdout
    }
};

static CustomLogger logger;

TEST(OBJECTPOOL_TESTS, ParserTest)
{
    using namespace isobus;

    isobus::CANStackLogger::set_can_stack_logger_sink(&logger);
    isobus::CANStackLogger::set_log_level(isobus::CANStackLogger::LoggingLevel::Debug);

    WorkingSetObject wso(nullptr);
    std::vector<std::uint8_t> workingsetBytes = {0xCD, 0xAB, 0x00, 0x02, 0x01, 0xe8, 0x03, 0x01, 0x00, 0x02, 0xf8, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x65, 0x6e, 0x64, 0x65,
                                                 0x00, 0xEF, 0x00, 0x02, 0x01, 0xe8, 0x03, 0x01, 0x00, 0x02, 0xf8, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x65, 0x6e, 0x64, 0x65};
    auto it = workingsetBytes.begin();
    bool res = wso.parse(workingsetBytes, it);

    EXPECT_EQ(static_cast<uint16_t>(wso.get_object_id()), workingsetBytes[0] | workingsetBytes[1] << 8);
    EXPECT_EQ(wso.get_object_type(), WorkingSetObject::ObjectType::WorkingSet);
    EXPECT_EQ(res, true);

    auto op = std::make_shared<ObjectPool>();
    EXPECT_TRUE(op->parse(workingsetBytes));
    auto workingset = op->get_object(std::int16_t(0xABCD));
    auto workingset2 = op->get_object(std::int16_t(0xEF00));
    ASSERT_EQ(workingset->get_object_type(), WorkingSetObject::ObjectType::WorkingSet);
    ASSERT_EQ(workingset2->get_object_type(), WorkingSetObject::ObjectType::WorkingSet);
    auto ws = std::static_pointer_cast<WorkingSetObject>(workingset);
}