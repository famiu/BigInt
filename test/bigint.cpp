#include "bigint/bigint.hpp"

#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

using namespace BI;

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

    SECTION("Invalid numbers")
    {
        REQUIRE_THROWS_AS(BigInt("1234567890a"), std::invalid_argument);
        REQUIRE_THROWS_AS(BigInt("-987654321a"), std::invalid_argument);
        REQUIRE_THROWS_AS(BigInt("0b2"), std::invalid_argument);
        REQUIRE_THROWS_AS(BigInt("0xg"), std::invalid_argument);
        REQUIRE_THROWS_AS(BigInt("0x"), std::invalid_argument);
        REQUIRE_THROWS_AS(BigInt(""), std::invalid_argument);
        REQUIRE_THROWS_AS(BigInt("-"), std::invalid_argument);
        REQUIRE_THROWS_AS(BigInt("-0x"), std::invalid_argument);
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

TEST_CASE("BigInt abs()")
{
    SECTION("Positive numbers")
    {
        REQUIRE((a).abs() == a);
        REQUIRE((b).abs() == b);
    }

    SECTION("Negative numbers")
    {
        REQUIRE((-a).abs() == a);
        REQUIRE((-b).abs() == b);
    }

    SECTION("Zero")
    {
        REQUIRE((0_bi).abs() == 0);
        REQUIRE((-0_bi).abs() == 0);
    }
}

TEST_CASE("BigInt to Integral conversion")
{
    SECTION("Positive numbers")
    {
        REQUIRE(static_cast<int>(a) == 1234567890);
        REQUIRE(static_cast<long long>(a) == 1234567890);
        REQUIRE(static_cast<unsigned long long>(a) == 1234567890);
    }

    SECTION("Negative numbers")
    {
        REQUIRE(static_cast<int>(-a) == -1234567890);
        REQUIRE(static_cast<long long>(-a) == -1234567890);
    }

    SECTION("Overflow/underflow")
    {
        REQUIRE_THROWS_AS(static_cast<int>(0x1234567890ABCDEF0123_bi), std::overflow_error);
        REQUIRE_THROWS_AS(static_cast<long long>(0x1234567890ABCDEF0123456789ABCDEF_bi), std::overflow_error);
        REQUIRE_THROWS_AS(static_cast<unsigned long long>(-1234_bi), std::underflow_error);
    }
}

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
    SECTION("Unary plus")
    {
        REQUIRE(+a == 1234567890_bi);
        REQUIRE(+b == 987654321_bi);
    }

    SECTION("Unary minus")
    {
        REQUIRE(-a == -1234567890_bi);
        REQUIRE(-b == -987654321_bi);
    }
}

TEST_CASE("BigInt Comparison")
{
    SECTION("Three-way comparison")
    {
        REQUIRE(a <=> b == std::strong_ordering::greater);
        REQUIRE(a <=> a == std::strong_ordering::equal);
        REQUIRE(b <=> a == std::strong_ordering::less);
    }

    SECTION("Equality and inequality")
    {
        REQUIRE(a == 1234567890);
        REQUIRE(a == 1234567890_bi);
        REQUIRE(1234567890 == a);
        REQUIRE(1234567890_bi == a);
        REQUIRE(a != b);
        REQUIRE(a != 987654321);
        REQUIRE(a != 987654321_bi);
        REQUIRE(987654321 != a);
        REQUIRE(987654321_bi != a);
    }

    SECTION("Greater and less than")
    {
        REQUIRE(a > b);
        REQUIRE(a >= b);
        REQUIRE(b < a);
        REQUIRE(b <= a);
    }

    SECTION("Negative three-way comparison")
    {
        REQUIRE(-a <=> -b == std::strong_ordering::less);
        REQUIRE(-a <=> -a == std::strong_ordering::equal);
        REQUIRE(-b <=> -a == std::strong_ordering::greater);
    }

    SECTION("Negative equality and inequality")
    {
        REQUIRE(-a == -1234567890);
        REQUIRE(-a == -1234567890_bi);
        REQUIRE(-1234567890 == -a);
        REQUIRE(-1234567890_bi == -a);
        REQUIRE(-a != -b);
        REQUIRE(-a != -987654321);
        REQUIRE(-a != -987654321_bi);
        REQUIRE(-987654321 != -a);
        REQUIRE(-987654321_bi != -a);
    }

    SECTION("Negative greater and less than")
    {
        REQUIRE(-a < -b);
        REQUIRE(-a <= -b);
        REQUIRE(-b > -a);
        REQUIRE(-b >= -a);
    }
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
    SECTION("Left shift")
    {
        REQUIRE(a << 1 == 2469135780_bi);
        REQUIRE(b << 1 == 1975308642_bi);
        REQUIRE(a << 10 == 1264197519360_bi);
        REQUIRE(b << 100 == 1252000592833654354567462788044156829696_bi);
    }

    SECTION("Right shift")
    {
        REQUIRE(a >> 1 == 617283945_bi);
        REQUIRE(b >> 1 == 493827160_bi);
        REQUIRE(a >> 10 == 1205632_bi);
        REQUIRE(a >> 100 == 0_bi);
    }
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
    BigInt c = 106048574244834508800_bi;
    BigInt d = -429391241160_bi;

    SECTION("BigInt Division")
    {
        REQUIRE(a / b == 1_bi);
        REQUIRE(a / (-b) == -1_bi);
        REQUIRE((-a) / b == -1_bi);
        REQUIRE((-a) / (-b) == 1_bi);
        REQUIRE(c / d == -246974237_bi);

        SECTION("Division by zero")
        {
            REQUIRE_THROWS_AS(a / 0_bi, std::domain_error);
        }
    }

    SECTION("BigInt Modulo")
    {
        REQUIRE(a % b == 246913569_bi);
        REQUIRE(a % (-b) == 246913569_bi);
        REQUIRE((-a) % b == -246913569_bi);
        REQUIRE((-a) % (-b) == -246913569_bi);
        REQUIRE(c % d == 84860513880_bi);

        SECTION("Division by zero")
        {
            REQUIRE_THROWS_AS(a % 0_bi, std::domain_error);
        }
    }
}

TEST_CASE("BigInt Power")
{
    SECTION("Zero power")
    {
        REQUIRE(a.pow(0) == 1_bi);
        REQUIRE((-a).pow(0) == 1_bi);
    }

    SECTION("Power of one")
    {
        REQUIRE(a.pow(1) == 1234567890_bi);
        REQUIRE((-a).pow(1) == -1234567890_bi);
    }

    SECTION("Positive powers")
    {
        REQUIRE(a.pow(2) == 1524157875019052100_bi);
        REQUIRE(a.pow(5) == 2867971860299718107233761438093672048294900000_bi);
        REQUIRE(
          a.pow(10) == 8225262591471025795047611436615355477641378922955141680937016996764162077997366010000000000_bi
        );
    }

    SECTION("Negative base positive powers")
    {
        REQUIRE((-a).pow(2) == 1524157875019052100_bi);
        REQUIRE((-a).pow(5) == -2867971860299718107233761438093672048294900000_bi);
    }

    SECTION("Edge cases")
    {
        REQUIRE((0_bi).pow(1000) == 0_bi);
        REQUIRE((1_bi).pow(1000) == 1_bi);
        REQUIRE((0_bi).pow(0) == 1_bi);
    }
}

TEST_CASE("BigInt std::format")
{
    SECTION("No format specifier")
    {
        REQUIRE(std::format("{}", 1234567890_bi) == "1234567890");
    }

    SECTION("Binary format")
    {
        REQUIRE(std::format("{:b}", 1234567890_bi) == "1001001100101100000001011010010");
        REQUIRE(std::format("{:b}", -1234567890_bi) == "-1001001100101100000001011010010");
        REQUIRE(std::format("{:#b}", 1234567890_bi) == "0b1001001100101100000001011010010");
        REQUIRE(std::format("{:#B}", 1234567890_bi) == "0B1001001100101100000001011010010");
    }

    SECTION("Octal format")
    {
        REQUIRE(std::format("{:o}", 1234567890_bi) == "11145401322");
        REQUIRE(std::format("{:o}", -1234567890_bi) == "-11145401322");
    }

    SECTION("Hexadecimal format")
    {
        REQUIRE(std::format("{:x}", 1234567890_bi) == "499602d2");
        REQUIRE(std::format("{:X}", 1234567890_bi) == "499602D2");
        REQUIRE(std::format("{:x}", -1234567890_bi) == "-499602d2");
        REQUIRE(std::format("{:X}", -1234567890_bi) == "-499602D2");
        REQUIRE(std::format("{:#x}", 1234567890_bi) == "0x499602d2");
        REQUIRE(std::format("{:#X}", 1234567890_bi) == "0X499602D2");
    }

    SECTION("Decimal format")
    {
        REQUIRE(std::format("{:d}", 1234567890_bi) == "1234567890");
        REQUIRE(std::format("{:d}", -1234567890_bi) == "-1234567890");
    }
}
