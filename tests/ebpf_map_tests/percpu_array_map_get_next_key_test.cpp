#include <gtest/gtest.h>

extern "C" {
#include <stdint.h>
#include <errno.h>

#include <dev/ebpf/ebpf_map.h>
}

namespace {
class PercpuArrayMapGetNextKeyTest : public ::testing::Test {
      protected:
	struct ebpf_map map;

	virtual void
	SetUp()
	{
		int error =
		    ebpf_map_init(&map, EBPF_MAP_TYPE_PERCPU_ARRAY, sizeof(uint32_t),
				  sizeof(uint32_t), 100, 0);
		ASSERT_TRUE(!error);
	}

	virtual void
	TearDown()
	{
		ebpf_map_deinit(&map, NULL);
	}
};

TEST_F(PercpuArrayMapGetNextKeyTest, GetNextKeyWithMaxKey)
{
	int error;
	uint32_t key = 99, next_key = 0;

	error = ebpf_map_get_next_key(&map, &key, &next_key);

	EXPECT_EQ(0, next_key);
}

TEST_F(PercpuArrayMapGetNextKeyTest, GetFirstKey)
{
	int error;
	uint32_t next_key = 0;

	error = ebpf_map_get_next_key(&map, NULL, &next_key);

	EXPECT_EQ(0, next_key);
}

TEST_F(PercpuArrayMapGetNextKeyTest, CorrectGetNextKey)
{
	int error;
	uint32_t key = 50, next_key = 0;

	error = ebpf_map_get_next_key(&map, &key, &next_key);

	EXPECT_EQ(51, next_key);
}
} // namespace
