#include "bigint.hpp"

#include <cassert>
#include <cmath>
#include <format>
#include <utility>

using ChunkType = BigInt::ChunkType;
using DataType = BigInt::DataType;

template<typename T>
static constexpr auto to_unsigned(T const &num) -> std::make_unsigned_t<T>
{
    if constexpr (std::is_unsigned_v<T>) {
        return num;
    } else {
        return static_cast<std::make_unsigned_t<T>>(num);
    }
}

template<typename T>
static constexpr auto to_signed(T const &num) -> std::make_signed_t<T>
{
    if constexpr (std::is_signed_v<T>) {
        return num;
    } else {
        if (num > std::numeric_limits<std::make_signed_t<T>>::max()) {
            throw std::overflow_error("Number is too large to be converted to a signed type");
        }
        return static_cast<std::make_signed_t<T>>(num);
    }
}

BigInt::BigInt()
{
    chunks.push_back(0);
}

BigInt::BigInt(std::integral auto const &num)
{
    auto const num_unsigned = to_unsigned(num);
    auto const num_size = sizeof(num_unsigned) * 8;

    for (auto i = 0; i < num_size; i += chunk_bits) {
        chunks.push_back(static_cast<ChunkType>((num_unsigned >> i) & chunk_max));
    }

    // Remove leading zeroes.
    remove_leading_zeroes();

    // Use two's complement if the number is negative.
    if (num < 0) {
        negative = true;
    }
}

BigInt::BigInt(std::string_view num)
{
    auto throw_invalid_number = [&num]() {
        throw std::invalid_argument(std::format("Invalid number: \"{}\"", num));
    };

    if (num.empty()) {
        throw_invalid_number();
    }

    negative = num[0] == '-';
    size_t index = negative ? 1z : 0z;
    Base base{Base::Decimal};

    if (num.size() > index + 1 && num[index] == '0') {
        if (std::tolower(num[index + 1]) == 'x') {
            base = Base::Hexadecimal;
            index += 2;
        } else if (std::tolower(num[index + 1]) == 'b') {
            base = Base::Binary;
            index += 2;
        } else {
            base = Base::Octal;
            index += 1;
        }
    }

    if (index >= num.size()) {
        throw_invalid_number();
    }

    // Convert the number to binary and store it in chunks.
    try {
        base_to_binary(num.substr(index), base);
    } catch (std::invalid_argument const &e) {
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
    if (is_zero()) {
        return rhs;
    }

    if (rhs.is_zero()) {
        return *this;
    }

    // Get number with more chunks.
    auto const &larger = chunks.size() > rhs.chunks.size() ? *this : rhs;
    auto const &smaller = chunks.size() > rhs.chunks.size() ? rhs : *this;

    BigInt result{larger};
    ChunkType carry = 0;

    for (size_t i = 0; i < smaller.chunks.size(); ++i) {
        if (result.chunks[i] < chunk_max - smaller.chunks[i] - carry) {
            result.chunks[i] += smaller.chunks[i] + carry;
            carry = 0;
        } else {
            // Add with overflow.
            result.chunks[i] += smaller.chunks[i] + carry;
            carry = 1;
        }
    }

    // Add carry to the rest of the larger number.
    if (carry != 0) {
        for (size_t i = smaller.chunks.size(); i < result.chunks.size(); ++i) {
            if (result.chunks[i] == chunk_max) {
                result.chunks[i] = 0;
            } else {
                result.chunks[i] += carry;
                carry = 0;
                break;
            }
        }
    }

    // Add carry to the end of the number.
    if (carry != 0) {
        result.chunks.push_back(1);
    }

    return result;
}

auto BigInt::operator-(BigInt const &rhs) const noexcept -> BigInt
{
    if (is_zero()) {
        return -rhs;
    }

    if (rhs.is_zero()) {
        return *this;
    }

    // Get larger number.
    bool const magnitude_greater = compare_magnitude(rhs) == std::strong_ordering::greater;
    auto const &larger = magnitude_greater ? *this : rhs;
    auto const &smaller = magnitude_greater ? rhs : *this;

    BigInt result{larger};
    result.negative = magnitude_greater ? negative : !negative;
    ChunkType borrow = 0;

    for (size_t i = 0; i < smaller.chunks.size(); ++i) {
        if (result.chunks[i] >= smaller.chunks[i] + borrow) {
            result.chunks[i] -= smaller.chunks[i] + borrow;
            borrow = 0;
        } else {
            // Subtract with underflow.
            result.chunks[i] -= smaller.chunks[i] + borrow;
            borrow = 1;
        }
    }

    // Subtract borrow from the rest of the larger number.
    if (borrow != 0) {
        for (size_t i = smaller.chunks.size(); i < result.chunks.size(); ++i) {
            if (result.chunks[i] == 0) {
                result.chunks[i] = chunk_max;
            } else {
                result.chunks[i] -= borrow;
                borrow = 0;
                break;
            }
        }
    }

    // Borrow cannot be 1 at the end of the number.
    assert(borrow == 0);

    // Remove leading zeroes.
    result.remove_leading_zeroes();

    return result;
}

auto BigInt::operator*(BigInt const &rhs) const noexcept -> BigInt
{
    if (is_zero() || rhs.is_zero()) {
        return BigInt{};
    }

    bool const magnitude_greater = compare_magnitude(rhs) == std::strong_ordering::greater;
    BigInt const &larger = magnitude_greater ? *this : rhs;
    BigInt const &smaller = magnitude_greater ? rhs : *this;

    BigInt result{};

    // Iterate through each bit of the smaller number in reverse order.
    // Shift the result by one bit and add the larger number to the result if the bit is set.
    for (size_t i = smaller.bit_count(); i-- > 0;) {
        result <<= 1;

        if (smaller.get_bit_at(i)) {
            result += larger;
        }
    }

    // Determine the sign of the result.
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

auto BigInt::operator&(BigInt const &rhs) const noexcept -> BigInt
{
    BigInt const &larger = chunks.size() > rhs.chunks.size() ? *this : rhs;
    BigInt const &smaller = chunks.size() > rhs.chunks.size() ? rhs : *this;
    BigInt result{larger};

    for (size_t i = 0; i < larger.chunks.size(); ++i) {
        if (i < smaller.chunks.size()) {
            result.chunks[i] = larger.chunks[i] & smaller.chunks[i];
        } else {
            result.chunks[i] = 0;
        }
    }

    result.remove_leading_zeroes();
    return result;
}

auto BigInt::operator|(BigInt const &rhs) const noexcept -> BigInt
{
    BigInt const &larger = chunks.size() > rhs.chunks.size() ? *this : rhs;
    BigInt const &smaller = chunks.size() > rhs.chunks.size() ? rhs : *this;
    BigInt result{larger};

    for (size_t i = 0; i < larger.chunks.size(); ++i) {
        if (i < smaller.chunks.size()) {
            result.chunks[i] = larger.chunks[i] | smaller.chunks[i];
        }
    }

    return result;
}

auto BigInt::operator^(BigInt const &rhs) const noexcept -> BigInt
{
    BigInt const &larger = chunks.size() > rhs.chunks.size() ? *this : rhs;
    BigInt const &smaller = chunks.size() > rhs.chunks.size() ? rhs : *this;
    BigInt result{larger};

    for (size_t i = 0; i < larger.chunks.size(); ++i) {
        if (i < smaller.chunks.size()) {
            result.chunks[i] = larger.chunks[i] ^ smaller.chunks[i];
        }
    }

    result.remove_leading_zeroes();
    return result;
}

auto BigInt::operator~() const noexcept -> BigInt
{
    BigInt result{*this};

    for (auto &chunk : result.chunks) {
        chunk = ~chunk;
    }

    result.remove_leading_zeroes();
    return result;
}

auto BigInt::operator<<(size_t rhs) const noexcept -> BigInt
{
    if (is_zero() || rhs == 0) {
        return *this;
    }

    BigInt result{*this};
    size_t chunk_shift = rhs / chunk_bits;
    size_t bit_shift = rhs % chunk_bits;

    result.chunks.insert(result.chunks.begin(), chunk_shift, 0);

    if (bit_shift != 0) {
        ChunkType carry = 0;

        for (size_t i = chunk_shift; i < result.chunks.size(); ++i) {
            ChunkType new_carry = result.chunks[i] >> (chunk_bits - bit_shift);
            result.chunks[i] = (result.chunks[i] << bit_shift) | carry;
            carry = new_carry;
        }

        if (carry != 0) {
            result.chunks.push_back(carry);
        }
    }

    return result;
}

auto BigInt::operator>>(size_t rhs) const noexcept -> BigInt
{
    if (is_zero() || rhs == 0) {
        return *this;
    }

    BigInt result{*this};
    size_t chunk_shift = rhs / chunk_bits;
    size_t bit_shift = rhs % chunk_bits;

    if (chunk_shift >= result.chunks.size()) {
        return BigInt{};
    }

    result.chunks.erase(result.chunks.begin(), std::next(result.chunks.begin(), to_signed(chunk_shift)));

    if (bit_shift != 0) {
        ChunkType carry = 0;

        for (size_t i = result.chunks.size(); i-- > 0;) {
            ChunkType new_carry = result.chunks[i] << (chunk_bits - bit_shift);
            result.chunks[i] = (result.chunks[i] >> bit_shift) | carry;
            carry = new_carry;
        }

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

auto BigInt::operator/=(BigInt const &rhs) noexcept -> BigInt &
{
    *this = *this / rhs;
    return *this;
}

auto BigInt::operator%=(BigInt const &rhs) noexcept -> BigInt &
{
    *this = *this % rhs;
    return *this;
}

auto BigInt::operator&=(BigInt const &rhs) noexcept -> BigInt &
{
    *this = *this & rhs;
    return *this;
}

auto BigInt::operator|=(BigInt const &rhs) noexcept -> BigInt &
{
    *this = *this | rhs;
    return *this;
}

auto BigInt::operator^=(BigInt const &rhs) noexcept -> BigInt &
{
    *this = *this ^ rhs;
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
    *this += 1_bi;
    return *this;
}

auto BigInt::operator--() noexcept -> BigInt &
{
    *this -= 1_bi;
    return *this;
}

auto BigInt::operator++(int) noexcept -> BigInt
{
    BigInt result{*this};
    *this += 1_bi;
    return result;
}

auto BigInt::operator--(int) noexcept -> BigInt
{
    BigInt result{*this};
    *this -= 1_bi;
    return result;
}

auto BigInt::operator<=>(BigInt const &rhs) const noexcept -> std::strong_ordering
{
    if (this->is_zero() && rhs.is_zero()) {
        return std::strong_ordering::equal;
    }

    if (negative != rhs.negative) {
        return negative ? std::strong_ordering::less : std::strong_ordering::greater;
    }

    return negative ? rhs.compare_magnitude(*this) : compare_magnitude(rhs);
}

BigInt::operator std::string() const
{
    return to_base(Base::Decimal);
}

auto BigInt::abs() const noexcept -> BigInt
{
    BigInt result{*this};
    result.negative = false;
    return result;
}

auto BigInt::convert(std::string const &output) noexcept -> bool
{
    try {
        *this = BigInt{output};
        return true;
    } catch (std::invalid_argument const &) {
        return false;
    }
}

auto BigInt::convert(std::integral auto const &output) noexcept -> bool
{
    try {
        *this = BigInt{output};
        return true;
    } catch (std::overflow_error const &) {
        return false;
    }
}

auto operator""_bi(char const *num) -> BigInt
{
    return BigInt{num};
}

auto BigInt::div(BigInt const &num, BigInt const &denom) -> std::pair<BigInt, BigInt>
{
    if (denom.is_zero()) {
        throw std::invalid_argument("Division by zero");
    }

    if (num.is_zero()) {
        return {BigInt{}, BigInt{}};
    }

    if (num.compare_magnitude(denom) == std::strong_ordering::less) {
        return {BigInt{}, num};
    }

    BigInt quotient{};
    BigInt remainder{num.abs()};

    while (remainder.compare_magnitude(denom) != std::strong_ordering::less) {
        BigInt temp{denom.abs()};
        size_t shift = remainder.bit_count() - temp.bit_count();

        temp <<= shift;

        while (temp.compare_magnitude(remainder) == std::strong_ordering::greater) {
            temp >>= 1;
            --shift;
        }

        remainder -= temp;
        quotient += 1_bi << shift;
    }

    // For remainder, the sign is the same as the dividend.
    remainder.negative = num.negative;
    quotient.negative = num.negative != denom.negative;

    return {quotient, remainder};
}

auto BigInt::bit_count() const -> size_t
{
    return (chunks.size() * chunk_bits) - static_cast<size_t>(std::countl_zero(chunks.back()));
}

/// Get the bit at the specified index.
///
/// @param index The index of the bit to get. 0th bit is the least significant bit and the last bit is the most
///              significant bit.
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

auto BigInt::is_negative() const -> bool
{
    return static_cast<bool>(chunks.back() >> (chunk_bits - 1));
}

/// Remove leading zeroes from the number.
void BigInt::remove_leading_zeroes()
{
    while (chunks.size() > 1 && chunks.back() == 0) {
        chunks.pop_back();
    }
}

auto BigInt::compare_magnitude(BigInt const &rhs) const noexcept -> std::strong_ordering
{
    if (chunks.size() != rhs.chunks.size()) {
        return chunks.size() <=> rhs.chunks.size();
    }

    for (size_t i = chunks.size(); i-- > 0;) {
        if (chunks[i] != rhs.chunks[i]) {
            return chunks[i] <=> rhs.chunks[i];
        }
    }

    return std::strong_ordering::equal;
}
