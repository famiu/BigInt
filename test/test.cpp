#include <catch2/catch_test_macros.hpp>

#include "bigint.hpp"

// Disable linting for Catch2 macros.
// NOLINTBEGIN(cppcoreguidelines-avoid-do-while,bugprone-chained-comparison)

TEST_CASE("BigInt default constructor")
{
    BigInt const a;
    REQUIRE(a == 0);
    REQUIRE(-a == 0);
    REQUIRE(a == 0_bi);
    REQUIRE(-a == 0_bi);
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
        REQUIRE(BigInt(9223372036854775807LL) == 9223372036854775807LL);
        REQUIRE(BigInt(18446744073709551615ULL) == 18446744073709551615ULL);
    }

    SECTION("Negative numbers")
    {
        REQUIRE(BigInt(-1234567890) == -1234567890);
        REQUIRE(BigInt(-987654321) == -987654321);
        REQUIRE(BigInt(-9223372036854775807LL) == -9223372036854775807LL);
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
        REQUIRE(BigInt("9223372036854775807") == 9223372036854775807LL);
        REQUIRE(BigInt("18446744073709551615") == 18446744073709551615ULL);
    }

    SECTION("Negative numbers")
    {
        REQUIRE(BigInt("-1234567890") == -1234567890);
        REQUIRE(BigInt("-987654321") == -987654321);
        REQUIRE(BigInt("-9223372036854775807") == -9223372036854775807LL);
    }

    SECTION("Prefixed numbers")
    {
        REQUIRE(BigInt("0x1234567890") == 0x1234567890);
        REQUIRE(BigInt("01234567") == 01234567);
        REQUIRE(BigInt("0b1010101") == 0b1010101);
        REQUIRE(BigInt("0x7fffffffffffffff") == 0x7fffffffffffffffLL);
        REQUIRE(BigInt("0xffffffffffffffff") == 0xffffffffffffffffULL);
    }

    SECTION("Negative prefixed numbers")
    {
        REQUIRE(BigInt("-0x1234567890") == -0x1234567890);
        REQUIRE(BigInt("-01234567") == -01234567);
        REQUIRE(BigInt("-0b1010101") == -0b1010101);
        REQUIRE(BigInt("-0x7fffffffffffffff") == -0x7fffffffffffffffLL);
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
        REQUIRE(9223372036854775807_bi == 9223372036854775807LL);
        REQUIRE(18446744073709551615_bi == 18446744073709551615ULL);
    }

    SECTION("Negative decimal numbers")
    {
        REQUIRE(-1234567890_bi == -1234567890);
        REQUIRE(-987654321_bi == -987654321);
        REQUIRE(-9223372036854775807_bi == -9223372036854775807LL);
    }

    SECTION("Prefixed numbers")
    {
        REQUIRE(0x1234567890_bi == 0x1234567890);
        REQUIRE(01234567_bi == 01234567);
        REQUIRE(0b1010101_bi == 0b1010101);
        REQUIRE(0x7fffffffffffffff_bi == 0x7fffffffffffffffLL);
        REQUIRE(0xffffffffffffffff_bi == 0xffffffffffffffffULL);
    }

    SECTION("Negative prefixed numbers")
    {
        REQUIRE(-0x1234567890_bi == -0x1234567890);
        REQUIRE(-01234567_bi == -01234567);
        REQUIRE(-0b1010101_bi == -0b1010101);
        REQUIRE(-0x7fffffffffffffff_bi == -0x7fffffffffffffffLL);
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

TEST_CASE("BigInt Comparison")
{
    REQUIRE(a <=> b == std::strong_ordering::greater);
    REQUIRE(a <=> a == std::strong_ordering::equal);
    REQUIRE(b <=> a == std::strong_ordering::less);
    REQUIRE(a == 1234567890);
    REQUIRE(a == 1234567890_bi);
    REQUIRE(1234567890 == a);
    REQUIRE(1234567890_bi == a);
    REQUIRE(a != b);
    REQUIRE(a != 987654321);
    REQUIRE(a != 987654321_bi);
    REQUIRE(987654321 != a);
    REQUIRE(987654321_bi != a);
    REQUIRE(a > b);
    REQUIRE(a >= b);
    REQUIRE(b < a);
    REQUIRE(b <= a);
    REQUIRE(-a <=> -b == std::strong_ordering::less);
    REQUIRE(-a <=> -a == std::strong_ordering::equal);
    REQUIRE(-b <=> -a == std::strong_ordering::greater);
    REQUIRE(-a == -1234567890);
    REQUIRE(-a == -1234567890_bi);
    REQUIRE(-1234567890 == -a);
    REQUIRE(-1234567890_bi == -a);
    REQUIRE(-a != -b);
    REQUIRE(-a != -987654321);
    REQUIRE(-a != -987654321_bi);
    REQUIRE(-987654321 != -a);
    REQUIRE(-987654321_bi != -a);
    REQUIRE(-a < -b);
    REQUIRE(-a <= -b);
    REQUIRE(-b > -a);
    REQUIRE(-b >= -a);
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

TEST_CASE("BigInt Bitshift")
{
    REQUIRE(a << 1 == 2469135780_bi);
    REQUIRE(a >> 1 == 617283945_bi);
    REQUIRE(b << 1 == 1975308642_bi);
    REQUIRE(b >> 1 == 493827160_bi);
    REQUIRE(a << 10 == 1264197519360_bi);
    REQUIRE(a >> 10 == 1205632_bi);
    REQUIRE(a >> 100 == 0_bi);
    REQUIRE(b << 100 == 1252000592833654354567462788044156829696_bi);
}

TEST_CASE("BigInt Multiplication")
{
    REQUIRE(a * b == 1219326311126352690_bi);
    REQUIRE(a * (-b) == -1219326311126352690_bi);
    REQUIRE((-a) * b == -1219326311126352690_bi);
    REQUIRE((-a) * (-b) == 1219326311126352690_bi);
}

TEST_CASE("BigInt Division and Modulo")
{
    REQUIRE(a / b == 1_bi);
    REQUIRE(a / (-b) == -1_bi);
    REQUIRE((-a) / b == -1_bi);
    REQUIRE((-a) / (-b) == 1_bi);
    REQUIRE(a % b == 246913569_bi);
    REQUIRE(a % (-b) == 246913569_bi);
    REQUIRE((-a) % b == -246913569_bi);
    REQUIRE((-a) % (-b) == -246913569_bi);

    BigInt c = 106048574244834508800_bi;
    BigInt d = -429391241160_bi;

    REQUIRE(c / d == -246974237_bi);
    REQUIRE(c % d == 84860513880_bi);
}

TEST_CASE("BigInt std::format")
{
    REQUIRE(std::format("{}", 1234567890_bi) == "1234567890");
    REQUIRE(std::format("{:b}", 1234567890_bi) == "1001001100101100000001011010010");
    REQUIRE(std::format("{:o}", 1234567890_bi) == "11145401322");
    REQUIRE(std::format("{:x}", 1234567890_bi) == "499602d2");
    REQUIRE(std::format("{:X}", 1234567890_bi) == "499602D2");
    REQUIRE(std::format("{:b}", -1234567890_bi) == "-1001001100101100000001011010010");
    REQUIRE(std::format("{:o}", -1234567890_bi) == "-11145401322");
    REQUIRE(std::format("{:x}", -1234567890_bi) == "-499602d2");
    REQUIRE(std::format("{:X}", -1234567890_bi) == "-499602D2");
    REQUIRE(std::format("{:#b}", 1234567890_bi) == "0b1001001100101100000001011010010");
    REQUIRE(std::format("{:#B}", 1234567890_bi) == "0B1001001100101100000001011010010");
    REQUIRE(std::format("{:#x}", 1234567890_bi) == "0x499602d2");
    REQUIRE(std::format("{:#X}", 1234567890_bi) == "0X499602D2");
}

// NOLINTEND(cppcoreguidelines-avoid-do-while,bugprone-chained-comparison)
