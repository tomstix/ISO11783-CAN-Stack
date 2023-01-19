#include <gtest/gtest.h>

#include <vector>
#include <memory>
#include <iostream>

#include "isobus/isobus/isobus_virtual_terminal_objects.hpp"

TEST(OBJECTPOOL_TESTS, ParserTest)
{
    using namespace isobus;

    WorkingSetObject wso;
    std::vector<std::uint8_t> workingsetBytes = {0x00, 0x01, 0x00, 0x02, 0x01, 0xe8, 0x03, 0x01, 0x00, 0x02, 0xf8, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x65, 0x6e, 0x64, 0x65};
    auto it = workingsetBytes.begin();
    bool res = wso.parse(workingsetBytes, it);

    EXPECT_EQ(static_cast<uint16_t>(wso.get_object_id()), workingsetBytes[0] | workingsetBytes[1] << 8);
    EXPECT_EQ(wso.get_object_type(), WorkingSetObject::ObjectType::WorkingSet);
    EXPECT_EQ(res, true);
}