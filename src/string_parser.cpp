#include <algorithm>
#include <cassert>
#include <cmath>
#include <format>
#include <stdexcept>
#include <utility>

#include "bigint.hpp"

using ChunkType = BigInt::ChunkType;
using DataType = BigInt::DataType;

// Character representation of all digits
static constexpr std::array<char, 16> digits = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static constexpr auto is_power_of_two(std::integral auto num) -> bool
{
    return num != 0 && (num & (num - 1)) == 0;
}

auto BigInt::is_valid_digit(Base base, char c) -> bool
{
    switch (base) {
    case Base::Binary:
        return c == '0' || c == '1';
    case Base::Octal:
        return c >= '0' && c <= '7';
    case Base::Decimal:
        return std::isdigit(c) != 0;
    case Base::Hexadecimal:
        return std::isxdigit(c) != 0;
    }
}

auto BigInt::char_to_digit(Base base, char c) -> ChunkType
{
    if (!is_valid_digit(base, c)) {
        throw std::invalid_argument(std::format("Invalid digit: {}", c));
    }

    switch (base) {
    case Base::Binary:
    case Base::Octal:
    case Base::Decimal:
        return static_cast<ChunkType>(c - '0');
    case Base::Hexadecimal:
        return static_cast<ChunkType>(std::isdigit(c) != 0 ? (c - '0') : (std::tolower(c) - 'a' + 10));
    }
}

/// Long divide string representation of a number by a divisor.
///
/// @param num The number to divide, must be unsigned.
/// @param[out] quotient Resulting quotient.
/// @param base The base of the number.
/// @param divisor The divisor.
auto BigInt::long_divide(std::string_view num, std::string &quotient, Base base, ChunkType divisor) -> ChunkType
{
    // Clear quotient string and reserve space.
    quotient.clear();
    quotient.reserve(num.size());

    // Numeric value of base.
    auto const base_num = std::to_underlying(base);
    ChunkType dividend = 0;

    for (char digit : num) {
        dividend = dividend * base_num + char_to_digit(base, digit);

        // If dividend >= divisor, divide and store the quotient
        if (dividend >= divisor) {
            ChunkType quot = dividend / divisor;
            ChunkType rem = dividend % divisor;

            assert(base_num < digits.size() && quot < base_num);  // Should always be true for valid bases.
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            quotient += digits[quot];
            dividend = rem;
        } else if (!quotient.empty()) {
            // If dividend is less than divisor and quotient is non-empty, add '0'.
            quotient += '0';
        }
    }

    return dividend;  // Return the remainder (which is the dividend now).
}

/// Convert string with power of two base to binary and store it in chunks.
///
/// @param num The number to convert, must be unsigned.
/// @param base The base of the number.
void BigInt::power_of_two_base_to_binary(std::string_view num, Base base)
{
    // Numeric value of base. Used for base conversion.
    auto const base_num = std::to_underlying(base);

    if (!is_power_of_two(base_num)) {
        throw std::invalid_argument("Base must be a power of 2");
    }

    // The number of bits needed to store a digit in the base.
    auto const bits_per_digit = static_cast<size_t>(std::countr_zero(base_num));

    // Clear the chunks vector.
    chunks.clear();

    ChunkType current_chunk{};
    size_t current_chunk_bits = 0;

    for (size_t i = num.size(); i > 0; i -= bits_per_digit) {
        // Bits that will fit in the current chunk.
        size_t const added_bits = std::min(bits_per_digit, chunk_bits - current_chunk_bits);
        // Bits that won't fit in the current chunk.
        size_t const remaining_bits = bits_per_digit - added_bits;
        // Digit corresponding to the current character.
        ChunkType const digit = char_to_digit(base, num[i - 1]);

        // Digit without the bits that won't fit in the current chunk.
        ChunkType const digit_masked = digit & ((1 << added_bits) - 1);

        // Add the bits corresponding to the digit to the current chunk.
        current_chunk = (digit_masked << current_chunk_bits) | current_chunk;

        // Increment the number of bits in the current chunk.
        current_chunk_bits += added_bits;

        if (current_chunk_bits == chunk_bits) {
            // The current chunk is full, push it to the chunks vector and reset the chunk.
            chunks.push_back(current_chunk);

            // Some bits may remain in the digit, add them to the new chunk.
            current_chunk = remaining_bits > 0 ? (digit >> added_bits) : 0;
            current_chunk_bits = remaining_bits;
        }
    }

    if (current_chunk_bits > 0) {
        // If the last chunk is not full, push it to the chunks vector.
        chunks.push_back(current_chunk);
    }
}

/// Convert decimal base to binary and store it in chunks.
///
/// @param num The number to convert, must be unsigned.
void BigInt::decimal_base_to_binary(std::string_view num)
{
    // Clear the chunks vector.
    chunks.clear();

    // Maximum number that can fit in a half-sized chunk.
    static constexpr auto const half_chunk_bits = chunk_bits / 2;

    // The process:
    // 1. Long divide the number by 2 ^ half_chunk_bits, the remainder will span half of the chunk. Only half of the
    //    chunk is processed at a time to avoid overflow.
    // 2. Every two iterations, push the full chunk to the chunks vector.
    // 3. Repeat until the number is 0.
    bool is_half_chunk = false;
    ChunkType current_chunk{0};
    static auto divisor = static_cast<ChunkType>(1) << half_chunk_bits;
    std::string current_num{num};
    std::string new_num;

    while (!current_num.empty()) {
        // If the previous iteration was the first half of the chunk, the current iteration will be the second half,
        // and vice versa.
        is_half_chunk = !is_half_chunk;

        // Long divide the current number by the divisor and store the remainder.
        // The resulting quotient is stored in the new_num string.
        ChunkType remainder = long_divide(current_num, new_num, Base::Decimal, divisor);

        // Store the remainder in the current chunk.
        if (is_half_chunk) {
            // First half of the chunk, just store the remainder.
            current_chunk = remainder;
        } else {
            // Full chunk, push it to the chunks vector and reset the chunk.
            // The most significant half of the chunk is the remainder from the previous iteration.
            chunks.push_back((current_chunk << half_chunk_bits) | remainder);
            current_chunk = 0;
        }

        // Swap the current and new num strings to avoid reallocations.
        std::swap(current_num, new_num);
    }

    // If the last chunk is not full, push it to the chunks vector.
    if (is_half_chunk) {
        chunks.push_back(current_chunk);
    }
}

/// Convert a base to binary and store it in chunks.
///
/// @param num The number to convert, must be unsigned.
/// @param base The base of the number.
void BigInt::base_to_binary(std::string_view num, Base base)
{
    // Check if base is a power of 2.
    if (is_power_of_two(std::to_underlying(base))) {
        power_of_two_base_to_binary(num, base);
    } else {
        assert(base == Base::Decimal);
        decimal_base_to_binary(num);
    }
}

auto BigInt::to_power_of_two_base(Base base) const -> std::string
{
    // Amount of bits that fit in a single digit of the specified base.
    auto const digit_bits = static_cast<ChunkType>(std::countr_zero(std::to_underlying(base)));
    std::string result;
    size_t bit_count = chunks.size() * chunk_bits;

    // Iterate through chunks of digit_bits bits and convert them to the specified base.
    for (size_t i = 0;; i += digit_bits) {
        // Number of bits to extract from the current chunk.
        size_t const bits_to_extract = std::min(digit_bits, bit_count - i);

        ChunkType digit = 0;
        for (size_t j = 0; j < bits_to_extract; ++j) {
            digit |= static_cast<ChunkType>(get_bit_at(i + j)) << j;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        result += digits[digit];
    }

    std::ranges::reverse(result);
    return result.empty() ? "0" : result;
}

auto BigInt::to_decimal() const -> std::string
{
    // Keep dividing modulo 10 and store the remainder in a string.
    BigInt quotient{*this};
    BigInt remainder;
    static const auto ten = 10_bi;
    std::string result;

    while (!quotient.is_zero()) {
        std::tie(quotient, remainder) = div(quotient, ten);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        result += digits[remainder.chunks[0]];
    }

    std::ranges::reverse(result);
    return result.empty() ? "0" : result;
}

auto BigInt::to_base(Base base) const -> std::string
{
    if (is_zero()) {
        return "0";
    }
    if (negative) {
        return "-" + (-(*this)).to_base(base);
    }
    if (is_power_of_two(std::to_underlying(base))) {
        return to_power_of_two_base(base);
    }
    assert(base == Base::Decimal);
    return to_decimal();
}
