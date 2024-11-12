#include <catch2/catch_test_macros.hpp>

#include "bigint.hpp"

TEST_CASE("BigInt Literals")
{
    SECTION("Zero")
    {
        REQUIRE(0_bi == 0);
        REQUIRE(-0_bi == 0);
    }

    SECTION("Positive decimal numbers")
    {
        REQUIRE(1234567890_bi == 1234567890);
        REQUIRE(987654321_bi == 987654321);
    }

    SECTION("Negative decimal numbers")
    {
        REQUIRE(-1234567890_bi == -1234567890);
        REQUIRE(-987654321_bi == -987654321);
    }

    SECTION("Literal prefixes")
    {
        REQUIRE(0x1234567890_bi == 0x1234567890);
        REQUIRE(01234567_bi == 01234567);
        REQUIRE(0b1010101_bi == 0b1010101);
    }

    SECTION("Negative literal prefixes")
    {
        REQUIRE(-0x1234567890_bi == -0x1234567890);
        REQUIRE(-01234567_bi == -01234567);
        REQUIRE(-0b1010101_bi == -0b1010101);
    }
}

TEST_CASE("BigInt String conversion")
{
    BigInt a = 1234567890_bi;
    BigInt b = 987654321_bi;

    SECTION("Positive numbers")
    {
        REQUIRE(std::string(a) == "1234567890");
        REQUIRE(std::string(b) == "987654321");
    }

    SECTION("Negative numbers")
    {
        REQUIRE(std::string(-a) == "-1234567890");
        REQUIRE(std::string(-b) == "-987654321");
    }
}

TEST_CASE("BigInt Unary operators")
{
    BigInt a = 1234567890_bi;
    BigInt b = 987654321_bi;

    REQUIRE(+a == 1234567890_bi);
    REQUIRE(-a == -1234567890_bi);
    REQUIRE(+b == 987654321_bi);
    REQUIRE(-b == -987654321_bi);
}

TEST_CASE("BigInt Addition")
{
    BigInt a = 1234567890_bi;
    BigInt b = 987654321_bi;

    BigInt c = a + b;
    REQUIRE(a + b == 2222222211_bi);
    // REQUIRE(a + (-b) == 246913569_bi);
    // REQUIRE((-a) + b == 246913569_bi);
    // REQUIRE((-a) + (-b) == -2222222211_bi);
}
