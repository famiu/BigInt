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

TEST_CASE("BigInt Copy constructor")
{
    BigInt const a = 1234567890_bi;
    BigInt const b = a;
    REQUIRE(a == b);
}

TEST_CASE("BigInt Move constructor")
{
    BigInt a = 1234567890_bi;
    BigInt b = std::move(a);
    REQUIRE(b == 1234567890);
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
BigInt const x = 883423532389192164791648750371459257913741948437_bi;
BigInt const y = 515377520732011331036461129765621272702107522001_bi;
BigInt const z = 368046011657180833755187620605837985211634426436515377520732011331036461129765621272702107522001_bi;

BigInt const a_neg = -1234567890_bi;
BigInt const b_neg = -987654321_bi;
BigInt const x_neg = -883423532389192164791648750371459257913741948437_bi;
BigInt const y_neg = -515377520732011331036461129765621272702107522001_bi;
BigInt const z_neg =
    -368046011657180833755187620605837985211634426436515377520732011331036461129765621272702107522001_bi;

std::string const a_str = "1234567890";
std::string const b_str = "987654321";
std::string const x_str = "883423532389192164791648750371459257913741948437";
std::string const y_str = "515377520732011331036461129765621272702107522001";
std::string const z_str =
    "368046011657180833755187620605837985211634426436515377520732011331036461129765621272702107522001";

TEST_CASE("BigInt abs()")
{
    SECTION("Positive numbers")
    {
        REQUIRE((a).abs() == a);
        REQUIRE((b).abs() == b);
        REQUIRE((x).abs() == x);
        REQUIRE((y).abs() == y);
    }

    SECTION("Negative numbers")
    {
        REQUIRE((-a).abs() == a);
        REQUIRE((-b).abs() == b);
        REQUIRE((-x).abs() == x);
        REQUIRE((-y).abs() == y);
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
        REQUIRE(std::string(a) == a_str);
        REQUIRE(std::string(b) == b_str);
        REQUIRE(std::string(x) == x_str);
        REQUIRE(std::string(y) == y_str);
        REQUIRE(std::string(z) == z_str);
    }

    SECTION("Negative numbers")
    {
        REQUIRE(std::string(-a) == "-" + a_str);
        REQUIRE(std::string(-b) == "-" + b_str);
        REQUIRE(std::string(-x) == "-" + x_str);
        REQUIRE(std::string(-y) == "-" + y_str);
        REQUIRE(std::string(-z) == "-" + z_str);
    }
}

TEST_CASE("BigInt Unary operators")
{
    SECTION("Unary plus")
    {
        REQUIRE(+a == a);
        REQUIRE(+b == b);
        REQUIRE(+x == x);
        REQUIRE(+y == y);
        REQUIRE(+z == z);
    }

    SECTION("Unary minus")
    {
        REQUIRE(-a == a_neg);
        REQUIRE(-b == b_neg);
        REQUIRE(-x == x_neg);
        REQUIRE(-y == y_neg);
        REQUIRE(-z == z_neg);
    }
}

TEST_CASE("BigInt Comparison")
{
    SECTION("Three-way comparison")
    {
        REQUIRE(x <=> y == std::strong_ordering::greater);
        REQUIRE(x <=> x == std::strong_ordering::equal);
        REQUIRE(y <=> x == std::strong_ordering::less);
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
        REQUIRE(x > y);
        REQUIRE(x >= y);
        REQUIRE(y < x);
        REQUIRE(y <= x);
    }

    SECTION("Negative three-way comparison")
    {
        REQUIRE(-x <=> -y == std::strong_ordering::less);
        REQUIRE(-x <=> -x == std::strong_ordering::equal);
        REQUIRE(-y <=> -x == std::strong_ordering::greater);
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
        REQUIRE(-x < -y);
        REQUIRE(-x <= -y);
        REQUIRE(-y > -x);
        REQUIRE(-y >= -x);
    }
}

TEST_CASE("BigInt Addition")
{
    REQUIRE(x + y == 1398801053121203495828109880137080530615849470438_bi);
    REQUIRE(x + (-y) == 368046011657180833755187620605837985211634426436_bi);
    REQUIRE((-x) + y == -368046011657180833755187620605837985211634426436_bi);
    REQUIRE((-x) + (-y) == -1398801053121203495828109880137080530615849470438_bi);
}

TEST_CASE("BigInt Subtraction")
{
    REQUIRE(x - y == 368046011657180833755187620605837985211634426436_bi);
    REQUIRE(x - (-y) == 1398801053121203495828109880137080530615849470438_bi);
    REQUIRE((-x) - y == -1398801053121203495828109880137080530615849470438_bi);
    REQUIRE((-x) - (-y) == -368046011657180833755187620605837985211634426436_bi);
}

TEST_CASE("BigInt Bitshift")
{
    SECTION("Left shift")
    {
        REQUIRE(x << 1 == 1766847064778384329583297500742918515827483896874_bi);
        REQUIRE(y << 1 == 1030755041464022662072922259531242545404215044002_bi);
        REQUIRE(x << 10 == 904625697166532776746648320380374280103671755199488_bi);
        REQUIRE(y << 100 == 653318623500070906096690267158057820537143710472954871543071966369497141477376_bi);
    }

    SECTION("Right shift")
    {
        REQUIRE(x >> 1 == 441711766194596082395824375185729628956870974218_bi);
        REQUIRE(y >> 1 == 257688760366005665518230564882810636351053761000_bi);
        REQUIRE(x >> 10 == 862718293348820473429344482784628181556388621_bi);
        REQUIRE(x >> 100 == 696898287454081973_bi);
        REQUIRE(x >> 1000 == 0);
    }
}

TEST_CASE("BigInt Multiplication")
{
    REQUIRE(
        x * y == 455296629879057568506896036293893753497585709776483642199615407824261470063242060286467585062437_bi
    );
    REQUIRE(
        x * (-y) == -455296629879057568506896036293893753497585709776483642199615407824261470063242060286467585062437_bi
    );
    REQUIRE(
        (-x) * y == -455296629879057568506896036293893753497585709776483642199615407824261470063242060286467585062437_bi
    );
    REQUIRE(
        (-x) * (-y)
        == 455296629879057568506896036293893753497585709776483642199615407824261470063242060286467585062437_bi
    );
}

TEST_CASE("BigInt Division and Modulo")
{
    BigInt c = 106048574244834508800_bi;
    BigInt d = -429391241160_bi;

    SECTION("BigInt Division")
    {
        REQUIRE(x / y == 1_bi);
        REQUIRE(x / (-y) == -1_bi);
        REQUIRE((-x) / y == -1_bi);
        REQUIRE((-x) / (-y) == 1_bi);
        REQUIRE(c / d == -246974237_bi);

        SECTION("Division by zero")
        {
            REQUIRE_THROWS_AS(a / 0_bi, std::domain_error);
        }
    }

    SECTION("BigInt Modulo")
    {
        REQUIRE(x % y == 368046011657180833755187620605837985211634426436_bi);
        REQUIRE(x % (-y) == 368046011657180833755187620605837985211634426436_bi);
        REQUIRE((-x) % y == -368046011657180833755187620605837985211634426436_bi);
        REQUIRE((-x) % (-y) == -368046011657180833755187620605837985211634426436_bi);
        REQUIRE(c % d == 84860513880_bi);

        SECTION("Division by zero")
        {
            REQUIRE_THROWS_AS(x % 0_bi, std::domain_error);
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
