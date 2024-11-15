#include "bigint/bigint.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <utility>
#ifdef _MSC_VER
#   include <intrin.h>
#endif

using namespace BI;
using namespace BI::detail;

// Avoid having to convert the number to a BigInt over and over again.
static auto const one = BigInt(1);

BigInt::BigInt()
{
    chunks.push_back(0);
}

BigInt::BigInt(std::string_view num)
{
    auto throw_invalid_number = [&num]()
    {
        throw std::invalid_argument(std::format("Invalid number: \"{}\"", num));
    };

    if (num.empty())
    {
        throw_invalid_number();
    }

    negative = num[0] == '-';
    size_t index = negative ? 1z : 0z;
    Base base{Base::Decimal};

    if (num.size() > index + 1 && num[index] == '0')
    {
        if (std::tolower(num[index + 1]) == 'x')
        {
            base = Base::Hexadecimal;
            index += 2;
        }
        else if (std::tolower(num[index + 1]) == 'b')
        {
            base = Base::Binary;
            index += 2;
        }
        else
        {
            base = Base::Octal;
            index += 1;
        }
    }

    if (index >= num.size())
    {
        throw_invalid_number();
    }

    // Convert the number to binary and store it in chunks.
    try
    {
        base_to_binary(num.substr(index), base);
    }
    catch (std::invalid_argument const &e)
    {
        throw_invalid_number();
    }

    // Remove leading zeroes.
    remove_leading_zeroes();
}

auto BigInt::operator+() const noexcept -> BigInt
{
    return *this;
}

auto BigInt::operator-() const noexcept -> BigInt
{
    BigInt result{*this};
    result.negative = !result.negative;
    return result;
}

auto BigInt::operator+(BigInt const &rhs) const noexcept -> BigInt
{
    if (is_zero())
    {
        return rhs;
    }

    if (rhs.is_zero())
    {
        return *this;
    }

    bool magnitude_greater = compare_magnitude(rhs) == std::strong_ordering::greater;
    BigInt result;

    if (negative == rhs.negative)
    {
        result = magnitude_greater ? add_magnitude(rhs) : rhs.add_magnitude(*this);
    }
    else
    {
        result = magnitude_greater ? subtract_magnitude(rhs) : rhs.subtract_magnitude(*this);
    }

    result.negative = magnitude_greater ? negative : rhs.negative;
    return result;
}

auto BigInt::operator-(BigInt const &rhs) const noexcept -> BigInt
{
    if (is_zero())
    {
        return -rhs;
    }

    if (rhs.is_zero())
    {
        return *this;
    }

    bool magnitude_greater = compare_magnitude(rhs) == std::strong_ordering::greater;
    BigInt result;

    if (negative == rhs.negative)
    {
        result = magnitude_greater ? subtract_magnitude(rhs) : rhs.subtract_magnitude(*this);
    }
    else
    {
        result = magnitude_greater ? add_magnitude(rhs) : rhs.add_magnitude(*this);
    }

    result.negative = magnitude_greater ? negative : !rhs.negative;
    return result;
}

auto BigInt::operator*(BigInt const &rhs) const noexcept -> BigInt
{
    if (is_zero() || rhs.is_zero())
    {
        return BigInt{};
    }
    if (*this == one)
    {
        return rhs;
    }
    if (rhs == one)
    {
        return *this;
    }

    bool const magnitude_greater = compare_magnitude(rhs) == std::strong_ordering::greater;
    BigInt const &larger = magnitude_greater ? *this : rhs;
    BigInt const &smaller = magnitude_greater ? rhs : *this;

    BigInt result{};
    // Reserve enough space for the result.
    // log(a * b) = log(a) + log(b).
    result.chunks.reserve(larger.chunks.size() + smaller.chunks.size());

    // Temporary result of multiplying the larger number by a single chunk of smaller number shifted to the left.
    BigInt temp{};
    temp.chunks.reserve(larger.chunks.size() + smaller.chunks.size());

    // Use grade school multiplication to multiply the numbers.
    // TODO(famiu): Use Karatsuba multiplication for large numbers.
    for (size_t i = 0; i < smaller.chunks.size(); ++i)
    {
        temp.chunks.clear();

        ChunkType carry = 0;
        for (ChunkType chunk : larger.chunks)
        {
            auto const [low, high] = multiply_chunks(chunk, smaller.chunks[i]);
            temp.chunks.push_back(low + carry);
            carry = high + static_cast<ChunkType>(low > chunk_max - carry);
        }

        if (carry != 0)
        {
            temp.chunks.push_back(carry);
        }

        temp.chunks.insert(temp.chunks.begin(), i, 0);
        result += temp;
    }

    result.negative = negative != rhs.negative;

    return result;
}

auto BigInt::operator/(BigInt const &rhs) const -> BigInt
{
    return div(*this, rhs).first;
}

auto BigInt::operator%(BigInt const &rhs) const -> BigInt
{
    return div(*this, rhs).second;
}

auto BigInt::operator<<(size_t rhs) const noexcept -> BigInt
{
    if (is_zero() || rhs == 0)
    {
        return *this;
    }

    BigInt result{*this};
    // Number of whole chunks to shift.
    size_t chunk_shift = rhs / chunk_bits;
    // Number of bits to shift within a chunk.
    size_t bit_shift = rhs % chunk_bits;

    // Add whole chunks of zeroes to the beginning of the number.
    result.chunks.reserve(result.chunks.size() + chunk_shift + 1);
    result.chunks.insert(result.chunks.begin(), chunk_shift, 0);

    // Shift the bits within the remaining chunks.
    if (bit_shift != 0)
    {
        ChunkType carry = 0;

        for (size_t i = chunk_shift; i < result.chunks.size(); ++i)
        {
            // Get the bits that will be shifted out of the current chunk and store them in carry.
            // Append the carry from the previous chunk to the current chunk.
            ChunkType new_carry = result.chunks[i] >> (chunk_bits - bit_shift);
            result.chunks[i] = (result.chunks[i] << bit_shift) | carry;
            carry = new_carry;
        }

        // If there is a carry left, add it to the end of the number.
        if (carry != 0)
        {
            result.chunks.push_back(carry);
        }
    }

    return result;
}

auto BigInt::operator>>(size_t rhs) const noexcept -> BigInt
{
    if (is_zero() || rhs == 0)
    {
        return *this;
    }

    BigInt result{*this};
    // Number of whole chunks to shift.
    size_t chunk_shift = rhs / chunk_bits;
    // Number of bits to shift within a chunk.
    size_t bit_shift = rhs % chunk_bits;

    // Shift is larger than the number of bits in the number, return 0.
    if (chunk_shift >= result.chunks.size())
    {
        return BigInt{};
    }

    // Erase the whole chunks that will be shifted.
    result.chunks.erase(result.chunks.begin(), std::next(result.chunks.begin(), to_signed(chunk_shift)));

    // Shift the bits within the remaining chunks.
    if (bit_shift != 0)
    {
        ChunkType carry = 0;

        for (size_t i = result.chunks.size(); i-- > 0;)
        {
            // Get the bits that will be shifted out of the current chunk and store them in carry.
            // Append the carry from the previous chunk to the current chunk.
            ChunkType new_carry = result.chunks[i] << (chunk_bits - bit_shift);
            result.chunks[i] = (result.chunks[i] >> bit_shift) | carry;
            carry = new_carry;
        }

        // Clear out any leading zero chunks that may have been created.
        result.remove_leading_zeroes();
    }

    return result;
}

auto BigInt::operator+=(BigInt const &rhs) noexcept -> BigInt &
{
    *this = *this + rhs;
    return *this;
}

auto BigInt::operator-=(BigInt const &rhs) noexcept -> BigInt &
{
    *this = *this - rhs;
    return *this;
}

auto BigInt::operator*=(BigInt const &rhs) noexcept -> BigInt &
{
    *this = *this * rhs;
    return *this;
}

auto BigInt::operator/=(BigInt const &rhs) -> BigInt &
{
    *this = *this / rhs;
    return *this;
}

auto BigInt::operator%=(BigInt const &rhs) -> BigInt &
{
    *this = *this % rhs;
    return *this;
}

auto BigInt::operator<<=(size_t rhs) noexcept -> BigInt &
{
    *this = *this << rhs;
    return *this;
}

auto BigInt::operator>>=(size_t rhs) noexcept -> BigInt &
{
    *this = *this >> rhs;
    return *this;
}

auto BigInt::operator++() noexcept -> BigInt &
{
    *this += one;
    return *this;
}

auto BigInt::operator--() noexcept -> BigInt &
{
    *this -= one;
    return *this;
}

auto BigInt::operator++(int) noexcept -> BigInt
{
    BigInt result{*this};
    *this += one;
    return result;
}

auto BigInt::operator--(int) noexcept -> BigInt
{
    BigInt result{*this};
    *this -= one;
    return result;
}

auto BigInt::operator<=>(BigInt const &rhs) const noexcept -> std::strong_ordering
{
    if (this->is_zero() && rhs.is_zero())
    {
        return std::strong_ordering::equal;
    }

    if (negative != rhs.negative)
    {
        return negative ? std::strong_ordering::less : std::strong_ordering::greater;
    }

    return negative ? rhs.compare_magnitude(*this) : compare_magnitude(rhs);
}

auto BigInt::operator==(BigInt const &rhs) const noexcept -> bool
{
    return (*this <=> rhs) == std::strong_ordering::equal;
}

BigInt::operator std::string() const
{
    return format_to_base(Base::Decimal);
}

auto BigInt::abs() const noexcept -> BigInt
{
    BigInt result{*this};
    result.negative = false;
    return result;
}

auto operator<<(std::ostream &os, BigInt const &num) -> std::ostream &
{
    return os << static_cast<std::string>(num);
}

auto operator""_bi(char const *num) -> BigInt
{
    return BigInt{num};
}

auto BigInt::div(BigInt const &num, BigInt const &denom) -> std::pair<BigInt, BigInt>
{
    if (denom.is_zero())
    {
        throw std::domain_error("Division by zero");
    }

    if (num.is_zero())
    {
        return {BigInt{}, BigInt{}};
    }

    if (num.compare_magnitude(denom) == std::strong_ordering::less)
    {
        return {BigInt{}, num};
    }

    BigInt quotient{};
    BigInt remainder{num.abs()};
    // Reserve enough space for the quotient.
    // log(a / b) = log(a) - log(b).
    quotient.chunks.reserve(num.chunks.size() - denom.chunks.size() + 1);

    // Perform long division:
    // 1. Find the largest multiple of the denominator that fits in the current remainder.
    // 2. Subtract the multiple from the remainder.
    // 3. Repeat until the remainder is less than the denominator.
    while (remainder.compare_magnitude(denom) != std::strong_ordering::less)
    {
        BigInt temp{denom.abs()};
        // Approximate the amount of shifts needed to align the most significant bit of the denominator with the
        // most significant bit of the remainder.
        size_t shift = remainder.bit_count() - temp.bit_count();

        // Align the most significant bit of the denominator with the most significant bit of the remainder.
        temp <<= shift;

        // If the denominator is still greater than the remainder, shift it to the right once.
        // This will guarantee that the denominator is less than the remainder.
        if (temp.compare_magnitude(remainder) == std::strong_ordering::greater)
        {
            temp >>= 1;
            --shift;
        }

        // Subtract the multiple from the remainder, and add the multiplier to the quotient.
        remainder -= temp;
        quotient += one << shift;
    }

    // For remainder, the sign is always the same as the dividend.
    remainder.negative = num.negative;
    quotient.negative = num.negative != denom.negative;

    return {quotient, remainder};
}

auto BigInt::pow(size_t power) const noexcept -> BigInt
{
    // x^0 = 1
    // NOTE: 0^0 also returns 1.
    if (power == 0)
    {
        return one;
    }

    // x^1 = x
    // 0^x = 0
    // 1^x = 1
    if (power == 1 || is_zero() || *this == one)
    {
        return *this;
    }

    static constexpr auto mask = static_cast<size_t>(1) << (sizeof(size_t) * 8 - 1);
    auto const power_leading_zeroes = static_cast<size_t>(std::countl_zero(power));
    // Get amount of bits in the power excluding leading zeroes.
    auto const power_bit_count = (sizeof(size_t) * 8) - power_leading_zeroes;

    BigInt result(1);
    // Reserve enough space for the result.
    // log(a ^ b) = b * log(a).
    result.chunks.reserve(chunks.size() * power);

    // Get rid of leading zeroes so that we can iterate through the bits of the power.
    power <<= power_leading_zeroes;

    // Iterate through the bits of the power, starting from the most significant bit.
    // Square the result in each iteration, and multiply it by the base if the bit is set.
    // After each iteration, left shift the power by 1 to get the next bit.
    for (size_t i = 0; i < power_bit_count; ++i)
    {
        result *= result;
        if ((power & mask) != 0)
        {
            result *= *this;
        }

        power <<= 1;
    }

    return result;
}

auto BigInt::bit_count() const -> size_t
{
    return (chunks.size() * chunk_bits) - static_cast<size_t>(std::countl_zero(chunks.back()));
}

auto BigInt::get_bit_at(size_t index) const -> bool
{
    size_t const chunk_index = index / chunk_bits;
    size_t const bit_index = index % chunk_bits;

    // Even though the chunks are stored in little endian, the bits in a chunk are stored in big endian.
    return static_cast<bool>((chunks[chunk_index] >> bit_index) & 1);
}

auto BigInt::is_zero() const -> bool
{
    return chunks.size() == 1 && chunks[0] == 0;
}

void BigInt::remove_leading_zeroes()
{
    while (chunks.size() > 1 && chunks.back() == 0)
    {
        chunks.pop_back();
    }
}

auto BigInt::compare_magnitude(BigInt const &rhs) const noexcept -> std::strong_ordering
{
    if (chunks.size() != rhs.chunks.size())
    {
        return chunks.size() <=> rhs.chunks.size();
    }

    for (size_t i = chunks.size(); i-- > 0;)
    {
        if (chunks[i] != rhs.chunks[i])
        {
            return chunks[i] <=> rhs.chunks[i];
        }
    }

    return std::strong_ordering::equal;
}

auto BigInt::add_magnitude(BigInt const &rhs) const noexcept -> BigInt
{
    assert(compare_magnitude(rhs) != std::strong_ordering::less);

    BigInt result{*this};
    ChunkType carry = 0;

    for (size_t i = 0; i < rhs.chunks.size(); ++i)
    {
        // Check if the current chunk can be added to without overflow.
        if (result.chunks[i] < chunk_max - rhs.chunks[i] - carry)
        {
            result.chunks[i] += rhs.chunks[i] + carry;
            carry = 0;
        }
        else
        {
            // Add with overflow and carry to next chunk.
            result.chunks[i] += rhs.chunks[i] + carry;
            carry = 1;
        }
    }

    // Add carry to the rest of the number.
    if (carry != 0)
    {
        for (size_t i = rhs.chunks.size(); i < result.chunks.size(); ++i)
        {
            if (result.chunks[i] == chunk_max)
            {
                result.chunks[i] = 0;
            }
            else
            {
                result.chunks[i] += carry;
                carry = 0;
                break;
            }
        }
    }

    // Add carry to the end of the number.
    if (carry != 0)
    {
        result.chunks.push_back(1);
    }

    return result;
}

auto BigInt::subtract_magnitude(BigInt const &rhs) const noexcept -> BigInt
{
    assert(compare_magnitude(rhs) != std::strong_ordering::less);

    BigInt result{*this};
    ChunkType borrow = 0;

    for (size_t i = 0; i < rhs.chunks.size(); ++i)
    {
        // Check if the current chunk can be subtracted from without underflow.
        if (result.chunks[i] >= rhs.chunks[i] + borrow)
        {
            result.chunks[i] -= rhs.chunks[i] + borrow;
            borrow = 0;
        }
        else
        {
            // Subtract with underflow and borrow from next chunk.
            result.chunks[i] -= rhs.chunks[i] + borrow;
            borrow = 1;
        }
    }

    // Subtract borrow from the rest of the number.
    if (borrow != 0)
    {
        for (size_t i = rhs.chunks.size(); i < result.chunks.size(); ++i)
        {
            if (result.chunks[i] == 0)
            {
                result.chunks[i] = chunk_max;
            }
            else
            {
                result.chunks[i] -= borrow;
                borrow = 0;
                break;
            }
        }
    }

    // Borrow cannot be 1 at the end of the number since rhs is smaller or equal.
    assert(borrow == 0);

    // Remove leading zeroes.
    result.remove_leading_zeroes();

    return result;
}

/// @details If the ChunkType is 32-bit or smaller, the result is calculated using 64-bit multiplication to prevent
/// overflow. For ChunkType of 64-bit size, if the compiler supports 128-bit integers, the result is calculated using
/// 128-bit multiplication. Otherwise, a custom 64-bit multiplication algorithm is used.
auto BigInt::multiply_chunks(ChunkType const a, ChunkType const b) noexcept -> std::pair<ChunkType, ChunkType>
{
    if constexpr (sizeof(ChunkType) <= 4)
    {
        // Use 64-bit multiplication to prevent overflow.
        auto const result = static_cast<uint64_t>(a) * b;
        return {static_cast<ChunkType>(result), static_cast<ChunkType>(result >> (sizeof(ChunkType) * 8))};
    }
    else
    {
        // Make sure that no weird-sized chunks are used.
        assert(sizeof(ChunkType) == 8);

#if defined(_MSC_VER) && defined(_M_X64)
        // Use MSVC intrinsics for 64-bit multiplication with overflow.
        uint64_t high;
        uint64_t low = _umul128(a, b, &high);
        return {static_cast<ChunkType>(low), static_cast<ChunkType>(high)};
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__SIZEOF_INT128__)
        // Use GCC/Clang extension for 128-bit multiplication.
        __uint128_t result = static_cast<__uint128_t>(a) * b;
        return {static_cast<ChunkType>(result), static_cast<ChunkType>(result >> 64)};
#else
        // Fall back to custom 64-bit multiplication.

        // Split the chunks into two halves and multiply them.
        constexpr auto half_chunk_mask = static_cast<ChunkType>(std::numeric_limits<uint32_t>::max());
        uint64_t const a1 = a >> 32;
        uint64_t const a0 = a & half_chunk_mask;
        uint64_t const b1 = b >> 32;
        uint64_t const b0 = b & half_chunk_mask;
        // a1a0 * b1b0 = (a1 * b1) * 2^64 + (a1 * b0 + a0 * b1) * 2^32 + a0 * b0
        uint64_t const p0 = a0 * b0;
        uint64_t const p1 = a1 * b0;
        uint64_t const p2 = a0 * b1;
        uint64_t const p3 = a1 * b1;

        // Higher 32 bits of the result (lower 32 bits are stored in p0).
        uint64_t const result_high = (p0 >> 32) + (p1 & half_chunk_mask) + (p2 & half_chunk_mask);
        // Overflow from the multiplication.
        uint64_t const overflow = p3 + (p1 >> 32) + (p2 >> 32) + (result_high >> 32);

        return {static_cast<ChunkType>((result_high << 32) | (p0 & half_chunk_mask)), static_cast<ChunkType>(overflow)};
#endif
    }
}
