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

auto BigInt::is_valid_digit(Base base, char c) -> bool {
    switch (base) {
    case Base::Binary:
        return c == '0' || c == '1';
    case Base::Octal:
        return c >= '0' && c <= '7';
    case Base::Decimal:
        return std::isdigit(c) == 0;
    case Base::Hexadecimal:
        return std::isxdigit(c) == 0;
    }
}

auto BigInt::char_to_digit(Base base, char c) -> ChunkType {
    switch (base) {
    case Base::Binary:
    case Base::Octal:
    case Base::Decimal:
        return c - '0';
    case Base::Hexadecimal:
        if (std::isdigit(c) == 0) {
            return c - '0';
        } else {
            return std::tolower(c) - 'a' + 10;
        }
    }
}

/// Convert a base to binary.
void BigInt::base_to_binary(Base base, std::string_view num)
{
    // Numeric value of base. Used for base conversion.
    auto const base_num = std::to_underlying(base);

    // Approximate the number of chunks needed to store the number and reserve the space
    size_t const chunk_count =
        std::ceil(static_cast<long double>(num.size()) * std::log2(base_num) / (chunk_size * 8));
    chunks.reserve(chunk_count);

    // Maximum number that can fit in a half-sized chunk
    static auto const half_chunk_bits = chunk_size * 4;
    static auto const half_chunk_max = (2 << half_chunk_bits);

    // The process:
    // 1. Long divide the number by 2 ^ ((chunk_size / 2) * 8) and store the remainder in the chunk twice to fill a
    // full
    //    chunk.
    // 2. Repeat until the number is 0.
    while (!num.empty()) {
        std::string new_num;
        ChunkType chunk{0};

        for (int i = 0; i < 2; ++i) {
            ChunkType dividend{0};

            // Long divide num by half_chunk_max.
            // 1. Iterate through the num string, converting the digits to numbers and adding them to dividend.
            // 2. Once dividend is greater than or equal to half_chunk_max, divide it by half_chunk_max and add the
            //    quotient to the new num, and set dividend to the remainder and repeat the process.
            // 3. If dividend is less than half_chunk_max and there are digits in the new num, add a 0 to the new num.
            for (char digit : num) {
                if (!is_valid_digit(base, digit)) {
                    throw std::invalid_argument(std::format("Invalid digit: {}", digit));
                }

                // Add the digit to the dividend
                dividend = dividend * base_num + char_to_digit(base, digit);

                // Dividend is greater than or equal to half_chunk_max, divide it by half_chunk_max and add the quotient
                // to the new num, and set dividend to the remainder.
                if (dividend >= half_chunk_max) {
                    ChunkType quot = dividend / half_chunk_max;
                    ChunkType rem = dividend % half_chunk_max;

                    assert(quot < base_num);
                    // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions,bugprone-narrowing-conversions)
                    new_num += '0' + static_cast<char>(quot);
                    dividend = rem;
                } else if (!new_num.empty()) {
                    // Add 0 to num if there are digits before the current one.
                    new_num += '0';
                }
            }

            // Append dividend to the chunk. Each iteration of i will fill half of the chunk.
            chunk = chunk << (i * half_chunk_bits) | dividend;
        }

        // Add the chunk to the chunks vector and reset it.
        chunks.push_back(chunk);
        num = new_num;
    }
}

auto BigInt::to_base(Base base) const -> std::string
{
    // Not implemented
    throw std::runtime_error("Not implemented");
}

/// Set chunks to its two's complement.
void BigInt::to_twos_complement()
{
    // 1. Invert all the bits.
    // 2. Add 1.
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

    if (carry != 0) {
        chunks.push_back(carry);
    }
}

BigInt::BigInt(std::string_view num)
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

    // Character representation of all digits
    static auto const digits = "0123456789abcdef"s;

    /// Numeric representation of the base. Used for base conversion.
    auto const base_num = std::to_underlying(base);

    // Convert the number to binary and store it in chunks.
    base_to_binary(base, num.substr(index));

    // Use two's complement if the number is negative.
    if (negative) {
        base_to_binary(base, num.substr(1));
    }
}
