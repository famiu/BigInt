#include <catch2/catch_test_macros.hpp>

#include "bigint.hpp"

TEST_CASE("BigInt")
{
    BigInt a = 1234567890_bi;
    BigInt b = 987654321_bi;

    SECTION("String conversion")
    {
        REQUIRE(std::string(a) == "1234567890");
        REQUIRE(std::string(b) == "987654321");
    }
}
