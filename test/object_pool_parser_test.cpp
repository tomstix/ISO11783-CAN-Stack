#include <gtest/gtest.h>

#include <vector>
#include <memory>
#include <iostream>

#include "isobus/isobus/isobus_virtual_terminal_objects.hpp"

TEST(OBJECTPOOL_TESTS, ParserTest)
{
    using namespace isobus;

    WorkingSetObject wso;
    std::vector<std::uint8_t> workingsetBytes = {0xCD, 0xAB, 0x00, 0x02, 0x01, 0xe8, 0x03, 0x01, 0x00, 0x02, 0xf8, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x65, 0x6e, 0x64, 0x65,
                                                 0x00, 0xEF, 0x00, 0x02, 0x01, 0xe8, 0x03, 0x01, 0x00, 0x02, 0xf8, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x65, 0x6e, 0x64, 0x65};
    auto it = workingsetBytes.begin();
    bool res = wso.parse(workingsetBytes, it);

    EXPECT_EQ(static_cast<uint16_t>(wso.get_object_id()), workingsetBytes[0] | workingsetBytes[1] << 8);
    EXPECT_EQ(wso.get_object_type(), WorkingSetObject::ObjectType::WorkingSet);
    EXPECT_EQ(res, true);

    auto op = std::make_shared<ObjectPool>();
    EXPECT_TRUE(op->parse(workingsetBytes));
    auto workingset = op->objects[std::int16_t(0xABCD)];
    auto workingset2 = op->objects[std::int16_t(0xEF00)];
    ASSERT_EQ(workingset->get_object_type(), WorkingSetObject::ObjectType::WorkingSet);
    ASSERT_EQ(workingset2->get_object_type(), WorkingSetObject::ObjectType::WorkingSet);
    auto ws = std::static_pointer_cast<WorkingSetObject>(workingset);
}