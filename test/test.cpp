#include <catch2/catch_test_macros.hpp>

#include "bigint.hpp"

TEST_CASE("BigInt default constructor")
{
    BigInt c;
    REQUIRE(c == 0);
    REQUIRE(-c == 0);
    REQUIRE(c == 0_bi);
    REQUIRE(-c == 0_bi);
}

TEST_CASE("BigInt integral constructor")
{
    SECTION("Zero")
    {
        REQUIRE(BigInt(0) == 0);
        REQUIRE(BigInt(-0) == 0);
    }

    SECTION("Positive numbers")
    {
        REQUIRE(BigInt(1234567890) == 1234567890);
        REQUIRE(BigInt(987654321) == 987654321);
    }

    SECTION("Negative numbers")
    {
        REQUIRE(BigInt(-1234567890) == -1234567890);
        REQUIRE(BigInt(-987654321) == -987654321);
    }
}

TEST_CASE("BigInt String constructor")
{
    SECTION("Zero")
    {
        REQUIRE(BigInt("0") == 0);
        REQUIRE(BigInt("-0") == 0);
    }

    SECTION("Positive numbers")
    {
        REQUIRE(BigInt("1234567890") == 1234567890);
        REQUIRE(BigInt("987654321") == 987654321);
    }

    SECTION("Negative numbers")
    {
        REQUIRE(BigInt("-1234567890") == -1234567890);
        REQUIRE(BigInt("-987654321") == -987654321);
    }

    SECTION("Prefixed numbers")
    {
        REQUIRE(BigInt("0x1234567890") == 0x1234567890);
        REQUIRE(BigInt("01234567") == 01234567);
        REQUIRE(BigInt("0b1010101") == 0b1010101);
    }

    SECTION("Negative prefixed numbers")
    {
        REQUIRE(BigInt("-0x1234567890") == -0x1234567890);
        REQUIRE(BigInt("-01234567") == -01234567);
        REQUIRE(BigInt("-0b1010101") == -0b1010101);
    }
}

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

    SECTION("Prefixed numbers")
    {
        REQUIRE(0x1234567890_bi == 0x1234567890);
        REQUIRE(01234567_bi == 01234567);
        REQUIRE(0b1010101_bi == 0b1010101);
    }

    SECTION("Negative prefixed numbers")
    {
        REQUIRE(-0x1234567890_bi == -0x1234567890);
        REQUIRE(-01234567_bi == -01234567);
        REQUIRE(-0b1010101_bi == -0b1010101);
    }
}

BigInt const a = 1234567890_bi;
BigInt const b = 987654321_bi;

TEST_CASE("BigInt to String conversion")
{
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
    REQUIRE(+a == 1234567890_bi);
    REQUIRE(-a == -1234567890_bi);
    REQUIRE(+b == 987654321_bi);
    REQUIRE(-b == -987654321_bi);
}

TEST_CASE("BigInt Addition")
{
    REQUIRE(a + b == 2222222211_bi);
    REQUIRE(a + (-b) == 246913569_bi);
    REQUIRE((-a) + b == -246913569_bi);
    REQUIRE((-a) + (-b) == -2222222211_bi);
}

TEST_CASE("BigInt Subtraction")
{
    REQUIRE(a - b == 246913569_bi);
    REQUIRE(a - (-b) == 2222222211_bi);
    REQUIRE((-a) - b == -2222222211_bi);
    REQUIRE((-a) - (-b) == -246913569_bi);
}
