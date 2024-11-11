#include "bigint.hpp"

#include <cassert>
#include <cmath>
#include <format>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

using namespace std::string_literals;

using ChunkType = BigInt::ChunkType;
using DataType = std::vector<ChunkType>;

constexpr auto chunk_size = sizeof(ChunkType);
constexpr ChunkType chunk_max = std::numeric_limits<ChunkType>::max();

enum class Base : std::uint_fast8_t
{
    Binary = 2,
    Octal = 8,
    Decimal = 10,
    Hexadecimal = 16
};

template<typename T>
static constexpr auto to_signed(T const &num) -> std::make_signed_t<T>
{
    if constexpr (std::is_signed_v<T>) {
        return num;
    } else {
        return static_cast<std::make_signed_t<T>>(num);
    }
}

template<typename T>
static constexpr auto to_unsigned(T const &num) -> std::make_unsigned_t<T>
{
    if constexpr (std::is_unsigned_v<T>) {
        return num;
    } else {
        return static_cast<std::make_unsigned_t<T>>(num);
    }
}

constexpr BigInt::BigInt()
{
    chunks.push_back(0);
}

constexpr BigInt::BigInt(std::integral auto const &num)
{
    constexpr auto num_unsigned = to_unsigned(num);
    constexpr auto num_size = sizeof(num_unsigned) * 8;

    for (auto i = 0; i < num_size; i += chunk_size * 8) {
        chunks.push_back(static_cast<ChunkType>((num_unsigned >> i) & chunk_max));
    }
}

BigInt::BigInt(std::string const &num)
{
    /// Prefix character after 0 that indicates the base of the number.
    static constexpr auto base_prefixes = {
        std::pair{'b', Base::Binary}, std::pair{'o', Base::Octal}, std::pair{'x', Base::Hexadecimal}
    };

    auto throw_invalid_number = [&num]() {
        throw std::invalid_argument(std::format("Invalid number: \"{}\"", num));
    };

    if (num.empty()) {
        throw_invalid_number();
    }

    bool negative = num[0] == '-';
    size_t index = negative ? 1z : 0z;
    Base base{Base::Decimal};

    if (num.size() > index + 1 && num[index] == '0') {
        for (auto const &[prefix, prefix_base] : base_prefixes) {
            if (num[index + 1] == prefix) {
                base = prefix_base;
                index += 2;
                break;
            }
        }
    }


    if (index >= num.size()) {
        throw_invalid_number();
    }

    /// Check if the character is a valid digit for the base.
    auto is_valid_digit = [base](char const &c) -> bool {
        switch (base) {
        case Base::Binary:
            return c == '0' || c == '1';
        case Base::Octal:
            return c >= '0' && c <= '7';
        case Base::Decimal:
            return std::isdigit(c);
        case Base::Hexadecimal:
            return std::isxdigit(c);
        }
    };

    /// Convert a character to a digit for the base.
    auto char_to_digit = [base](char const &c) -> ChunkType {
        switch (base) {
        case Base::Binary:
        case Base::Octal:
        case Base::Decimal:
            return c - '0';
        case Base::Hexadecimal:
            if (std::isdigit(c)) {
                return c - '0';
            } else {
                return std::tolower(c) - 'a' + 10;
            }
        }
    };

    // Character representation of all digits
    static auto const digits = "0123456789abcdef"s;

    /// Numeric representation of the base. Used for base conversion.
    auto const base_num = std::to_underlying(base);

    // Approximate the number of chunks needed to store the number and reserve the space
    size_t const chunk_count =
        std::ceil(static_cast<long double>(num.size()) * std::log2(base_num) / (chunk_size * 8));
    chunks.reserve(chunk_count);

    // The process:
    // 1. Long divide the number by 2 ^ ((chunk_size / 2) * 8) and store the remainder in the chunk twice to fill a
    // full
    //    chunk.
    // 2. Repeat until the number is 0
    // 3. If the number is negative, take the two's complement by inverting the bits and adding 1
    ChunkType chunk{0};
    std::string result = num.substr(index);

    // Maximum number that can fit in a half-sized chunk
    static auto const half_chunk_bits = chunk_size * 4;
    static auto const half_chunk_max = (2 << half_chunk_bits);

    while (result != "") {
        std::string new_result;
        ChunkType carry{0};

        for (int i = 0; i < 2; ++i) {
            ChunkType dividend{0};

            // Long divide result by half_chunk_max.
            // 1. Iterate through the result string, converting the digits to numbers and adding them to dividend.
            // 2. Once dividend is greater than or equal to half_chunk_max, divide it by half_chunk_max and add the
            //    quotient to the new result, and set dividend to the remainder and repeat the process.
            // 3. If dividend is less than half_chunk_max and there are digits in the new result, add a 0 to the new
            //    result.
            for (char digit : result) {
                if (!is_valid_digit(digit)) {
                    throw_invalid_number();
                }

                // Add the digit to the dividend
                dividend = dividend * base_num + char_to_digit(digit);

                // Dividend is greater than or equal to half_chunk_max, divide it by half_chunk_max and add the quotient
                // to the new result, and set dividend to the remainder.
                if (dividend >= half_chunk_max) {
                    ChunkType quot = dividend / half_chunk_max;
                    ChunkType rem = dividend % half_chunk_max;

                    assert(quot < base_num);
                    new_result += digits[quot];
                    dividend = rem;
                } else if (!new_result.empty()) {
                    // Add 0 to result if there are digits before the current one.
                    new_result += '0';
                }
            }

            // Append dividend to the chunk. Each iteration of i will fill half of the chunk.
            chunk = chunk << (i * half_chunk_bits) | dividend;
        }

        // Add the chunk to the chunks vector and reset it.
        chunks.push_back(chunk);
        chunk = 0;
        result = new_result;
    }

    // Use two's complement if the number is negative.
    // 1. Invert all the bits.
    // 2. Add 1.
    if (negative) {
        for (auto &c : chunks) {
            c = ~c;
        }

        ChunkType carry{1};
        for (auto &c : chunks) {
            if (c == chunk_max && carry != 0) {
                c = 0;
            } else {
                c += carry;
                carry = 0;
            }
        }
    }
}
