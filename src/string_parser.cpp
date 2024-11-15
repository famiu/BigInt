#include <algorithm>
#include <cassert>
#include <cmath>
#include <format>
#include <stdexcept>
#include <utility>

#include "bigint/bigint.hpp"

using namespace BI;
using namespace BI::detail;

static auto const log2_10 = std::log2(10);

// Character representation of all digits
static constexpr std::array digits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

// Character representation of all digits in lowercase.
static constexpr std::array digits_lowercase = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

static constexpr auto is_power_of_two(std::integral auto num) -> bool
{
    return num != 0 && (num & (num - 1)) == 0;
}

auto BigInt::is_valid_digit(Base base, char c) -> bool
{
    switch (base)
    {
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
    if (!is_valid_digit(base, c))
    {
        throw std::invalid_argument(std::format("Invalid digit: {}", c));
    }

    switch (base)
    {
    case Base::Binary:
    case Base::Octal:
    case Base::Decimal:
        return static_cast<ChunkType>(c - '0');
    case Base::Hexadecimal:
        return static_cast<ChunkType>(std::isdigit(c) != 0 ? (c - '0') : (std::tolower(c) - 'a' + 10));
    }
}

auto BigInt::long_divide(std::string_view num, std::string &quotient, Base base, ChunkType divisor) -> ChunkType
{
    // Clear quotient string and reserve space.
    quotient.clear();
    quotient.reserve(num.size());

    // Numeric value of base.
    auto const base_num = std::to_underlying(base);
    ChunkType dividend = 0;

    for (char digit : num)
    {
        dividend = dividend * base_num + char_to_digit(base, digit);

        // If dividend >= divisor, divide and store the quotient
        if (dividend >= divisor)
        {
            ChunkType quot = dividend / divisor;
            ChunkType rem = dividend % divisor;

            assert(base_num < digits.size() && quot < base_num);  // Should always be true for valid bases.
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            quotient += digits[quot];
            dividend = rem;
        }
        else if (!quotient.empty())
        {
            // If dividend is less than divisor and quotient is non-empty, add '0'.
            quotient += '0';
        }
    }

    return dividend;  // Return the remainder (which is the dividend now).
}

void BigInt::power_of_two_base_to_binary(std::string_view num, Base base)
{
    // Numeric value of base. Used for base conversion.
    auto const base_num = std::to_underlying(base);

    if (!is_power_of_two(base_num))
    {
        throw std::invalid_argument("Base must be a power of 2");
    }

    // The number of bits needed to store a digit in the base.
    auto const bits_per_digit = static_cast<size_t>(std::countr_zero(base_num));

    // Clear the chunks vector and reserve space.
    chunks.clear();
    chunks.reserve((num.size() * bits_per_digit / chunk_bits) + 1);

    ChunkType current_chunk{};
    size_t current_chunk_bits = 0;

    for (size_t i = num.size(); i-- > 0;)
    {
        // Bits that will fit in the current chunk.
        size_t const added_bits = std::min(bits_per_digit, chunk_bits - current_chunk_bits);
        // Bits that won't fit in the current chunk.
        size_t const remaining_bits = bits_per_digit - added_bits;
        // Digit corresponding to the current character.
        ChunkType const digit = char_to_digit(base, num[i]);

        // Digit without the bits that won't fit in the current chunk.
        ChunkType const digit_masked = digit & ((1 << added_bits) - 1);

        // Add the bits corresponding to the digit to the current chunk.
        current_chunk = (digit_masked << current_chunk_bits) | current_chunk;

        // Increment the number of bits in the current chunk.
        current_chunk_bits += added_bits;

        if (current_chunk_bits == chunk_bits)
        {
            // The current chunk is full, push it to the chunks vector and reset the chunk.
            chunks.push_back(current_chunk);

            // Some bits may remain in the digit, add them to the new chunk.
            current_chunk = remaining_bits > 0 ? (digit >> added_bits) : 0;
            current_chunk_bits = remaining_bits;
        }
    }

    if (current_chunk_bits > 0)
    {
        // If the last chunk is not full, push it to the chunks vector.
        chunks.push_back(current_chunk);
    }
}

void BigInt::decimal_base_to_binary(std::string_view num)
{
    // Clear the chunks vector and reserve space.
    chunks.clear();
    chunks.reserve(static_cast<size_t>(static_cast<long double>(num.size()) * log2_10 / chunk_bits) + 1);

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

    while (!current_num.empty())
    {
        // If the previous iteration was the first half of the chunk, the current iteration will be the second half,
        // and vice versa.
        is_half_chunk = !is_half_chunk;

        // Long divide the current number by the divisor and store the remainder.
        // The resulting quotient is stored in the new_num string.
        ChunkType remainder = long_divide(current_num, new_num, Base::Decimal, divisor);

        // Store the remainder in the current chunk.
        if (is_half_chunk)
        {
            // First half of the chunk, just store the remainder.
            current_chunk = remainder;
        }
        else
        {
            // Full chunk, push it to the chunks vector and reset the chunk.
            // The most significant half of the chunk is the remainder from this iteration.
            chunks.push_back((remainder << half_chunk_bits) | current_chunk);
            current_chunk = 0;
        }

        // Swap the current and new num strings to avoid reallocations.
        std::swap(current_num, new_num);
    }

    // If the last chunk is not full, push it to the chunks vector.
    if (is_half_chunk)
    {
        chunks.push_back(current_chunk);
    }
}

void BigInt::base_to_binary(std::string_view num, Base base)
{
    if (is_power_of_two(std::to_underlying(base)))
    {
        power_of_two_base_to_binary(num, base);
    }
    else
    {
        assert(base == Base::Decimal);
        decimal_base_to_binary(num);
    }
}

auto BigInt::format_to_power_of_two_base(Base base, bool add_prefix, bool capitalize) const noexcept -> std::string
{
    // Amount of bits that fit in a single digit of the specified base.
    auto const digit_bits = static_cast<size_t>(std::countr_zero(std::to_underlying(base)));
    size_t bit_count = this->bit_count();
    std::string result;
    // Reserve enough space for the result.
    result.reserve((bit_count / digit_bits) + 1 + (add_prefix ? 2 : 0));

    // Iterate through chunks of digit_bits bits and convert them to the specified base.
    size_t i = 0;
    while (i < bit_count)
    {
        // Number of bits to extract from the current chunk.
        size_t const extracted_bits = std::min(digit_bits, bit_count - i);

        ChunkType digit = 0;
        for (size_t j = 0; j < extracted_bits; ++j)
        {
            digit |= static_cast<ChunkType>(get_bit_at(i + j)) << j;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        result += capitalize ? digits[digit] : digits_lowercase[digit];

        i += extracted_bits;
    }

    // Reverse the result string to make it MSB first.
    std::ranges::reverse(result);

    if (result.empty())
    {
        result = "0";
    }

    std::string prefix;

    if (add_prefix)
    {
        switch (base)
        {
        case Base::Binary:
            prefix = capitalize ? "0B" : "0b";
            break;
        case Base::Octal:
            prefix = "0";
            break;
        case Base::Hexadecimal:
            prefix = capitalize ? "0X" : "0x";
            break;
        case Base::Decimal:
            assert(false);  // Should never happen.
        }
    }

    result.insert(0, prefix);
    return result;
}

auto BigInt::format_to_decimal() const -> std::string
{
    BigInt quotient{*this};
    BigInt remainder;
    static auto const ten = BigInt(10);
    std::string result;
    // Reserve enough space for the result.
    result.reserve(static_cast<size_t>(std::ceil(static_cast<long double>(quotient.bit_count()) / log2_10) + 1));

    // Keep dividing modulo 10 and store the remainder in a string.
    while (!quotient.is_zero())
    {
        std::tie(quotient, remainder) = div(quotient, ten);
        // Append the remainder to the result string.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        result += digits[remainder.chunks[0]];
    }

    std::ranges::reverse(result);
    return result.empty() ? "0" : result;
}

auto BigInt::format_to_base(Base base, bool add_prefix, bool capitalize) const -> std::string
{
    if (is_zero())
    {
        return "0";
    }
    if (negative)
    {
        return "-" + (-(*this)).format_to_base(base, add_prefix, capitalize);
    }
    if (is_power_of_two(std::to_underlying(base)))
    {
        return format_to_power_of_two_base(base, add_prefix, capitalize);
    }
    assert(base == Base::Decimal);
    return format_to_decimal();
}
