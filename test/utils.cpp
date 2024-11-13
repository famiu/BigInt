#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <limits>
#include <stdexcept>

using namespace BI::detail;

// Disable linting for Catch2 macros.
// NOLINTBEGIN(cppcoreguidelines-avoid-do-while,bugprone-chained-comparison)

TEST_CASE("to_unsigned()")
{
    SECTION("Unsigned type")
    {
        REQUIRE(to_unsigned(0U) == 0U);
        REQUIRE(to_unsigned(1U) == 1U);
        REQUIRE(to_unsigned(42U) == 42U);
        REQUIRE(to_unsigned(0xFFFFFFFFU) == 0xFFFFFFFFU);
    }

    SECTION("Positive signed type")
    {
        REQUIRE(to_unsigned(0L) == 0U);
        REQUIRE(to_unsigned(1L) == 1U);
        REQUIRE(to_unsigned(42L) == 42U);
        REQUIRE(to_unsigned(std::numeric_limits<long>::max()) == std::numeric_limits<long>::max());
    }

    SECTION("Negative signed type")
    {
        REQUIRE_THROWS_AS(to_unsigned(-42L), std::underflow_error);
    }
}

TEST_CASE("to_signed()")
{
    SECTION("Unsigned type")
    {
        REQUIRE(to_signed(0U) == 0L);
        REQUIRE(to_signed(1U) == 1L);
        REQUIRE(to_signed(42U) == 42L);
    }

    SECTION("Positive signed type")
    {
        REQUIRE(to_signed(0L) == 0L);
        REQUIRE(to_signed(1L) == 1L);
        REQUIRE(to_signed(42L) == 42L);
        REQUIRE(to_signed(0xFFFFFFFFL) == 0xFFFFFFFFL);
    }

    SECTION("Negative signed type")
    {
        REQUIRE(to_signed(-42L) == -42L);
        REQUIRE(to_signed(std::numeric_limits<long>::min()) == std::numeric_limits<long>::min());
    }

    SECTION("Overflow")
    {
        REQUIRE_THROWS_AS(to_signed(std::numeric_limits<unsigned long>::max()), std::overflow_error);
    }
}

TEST_CASE("type_name()")
{
    REQUIRE(type_name<int>() == "int");
    REQUIRE(type_name<long>() == "long");
    REQUIRE(type_name<unsigned long>() == "unsigned long");
    REQUIRE(type_name<std::string>() == "std::basic_string<char>");
    REQUIRE(type_name<std::vector<int>>() == "std::vector<int>");
}

// NOLINTEND(cppcoreguidelines-avoid-do-while,bugprone-chained-comparison)
