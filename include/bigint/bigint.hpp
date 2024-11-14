#pragma once

#include <concepts>
#include <cstdint>
#include <format>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "utils.hpp"

namespace BI
{
class BigInt
{
public:
    BigInt();
    BigInt(BigInt const &rhs) = default;
    BigInt(BigInt &&rhs) noexcept = default;

    explicit BigInt(std::integral auto const &num) noexcept : negative{num < 0}
    {
        auto const num_unsigned = detail::to_unsigned(negative ? -num : num);
        auto const num_size = sizeof(num_unsigned) * 8;

        for (size_t i = 0; i < num_size; i += chunk_bits) {
            chunks.push_back(static_cast<ChunkType>((num_unsigned >> i) & chunk_max));
        }

        // Remove leading zeroes.
        remove_leading_zeroes();
    }

    explicit BigInt(std::string_view num);
    ~BigInt() = default;

    auto operator=(BigInt const &rhs) noexcept -> BigInt & = default;
    auto operator=(BigInt &&rhs) noexcept -> BigInt & = default;

    auto operator+() const noexcept -> BigInt;
    auto operator-() const noexcept -> BigInt;

    auto operator+(BigInt const &rhs) const noexcept -> BigInt;
    auto operator-(BigInt const &rhs) const noexcept -> BigInt;
    auto operator*(BigInt const &rhs) const noexcept -> BigInt;
    auto operator/(BigInt const &rhs) const -> BigInt;
    auto operator%(BigInt const &rhs) const -> BigInt;

    auto operator<<(size_t rhs) const noexcept -> BigInt;
    auto operator>>(size_t rhs) const noexcept -> BigInt;

    auto operator+=(BigInt const &rhs) noexcept -> BigInt &;
    auto operator-=(BigInt const &rhs) noexcept -> BigInt &;
    auto operator*=(BigInt const &rhs) noexcept -> BigInt &;
    auto operator/=(BigInt const &rhs) -> BigInt &;
    auto operator%=(BigInt const &rhs) -> BigInt &;

    auto operator<<=(size_t rhs) noexcept -> BigInt &;
    auto operator>>=(size_t rhs) noexcept -> BigInt &;

    auto operator++() noexcept -> BigInt &;
    auto operator--() noexcept -> BigInt &;
    auto operator++(int) noexcept -> BigInt;
    auto operator--(int) noexcept -> BigInt;

    auto operator<=>(BigInt const &rhs) const noexcept -> std::strong_ordering;
    auto operator==(BigInt const &rhs) const noexcept -> bool;

    auto operator<=>(std::integral auto const &rhs) const noexcept -> std::strong_ordering
    {
        try {
            return static_cast<decltype(rhs)>(*this) <=> rhs;
        } catch (std::overflow_error const &) {
            return negative ? std::strong_ordering::less : std::strong_ordering::greater;
        }
    }

    auto operator==(std::integral auto const &rhs) const noexcept -> bool
    {
        return (*this <=> rhs) == std::strong_ordering::equal;
    }

    template<std::integral T>
    explicit operator T() const
    {
        using UnsignedT = std::make_unsigned_t<T>;

        constexpr auto is_signed = std::is_signed_v<T>;
        size_t const num_bits = this->bit_count();

        // Unsigned types cannot store negative numbers.
        if (negative && !is_signed) {
            throw std::underflow_error(std::format("Number can't fit in unsigned type '{}'", detail::type_name<T>()));
        }
        // Signed types can store 1 less bit than their signed counterpart.
        if (num_bits > (sizeof(T) * 8) - static_cast<size_t>(is_signed)) {
            throw std::overflow_error(  // clang-format off
                    std::format("Number is too large to be converted to type '{}'", detail::type_name<T>())
            );  // clang-format on
        }

        UnsignedT result{};

        for (size_t i = 0; i < sizeof(T) * 8; i += chunk_bits) {
            result |= static_cast<T>(this->chunks[i / chunk_bits]) << i;
        }

        return negative ? -static_cast<T>(result) : static_cast<T>(result);
    }

    explicit operator std::string() const;

    /// @brief Get the absolute value of the number.
    [[nodiscard]] auto abs() const noexcept -> BigInt;

    /// @brief Convert the number to the specified type.
    ///
    /// @param[out] output Result of the conversion.
    /// @return Whether the conversion was successful.
    [[nodiscard]] auto convert(std::string &output) const noexcept -> bool
    {
        try {
            output = static_cast<std::string>(*this);
            return true;
        } catch (std::invalid_argument const &) {
            return false;
        }
    }

    /// @brief Convert the number to the specified type.
    ///
    /// @tparam T The type to convert the number to.
    /// @param[out] output Result of the conversion.
    /// @return Whether the conversion was successful.
    [[nodiscard]] auto convert(std::integral auto &output) const noexcept -> bool
    {
        try {
            output = static_cast<decltype(output)>(*this);
            return true;
        } catch (std::overflow_error const &) {
            return false;
        }
    }

    /// @brief Divide two numbers and return the quotient and remainder.
    ///
    /// @param num The dividend.
    /// @param denom The divisor.
    /// @return The quotient and remainder.
    ///
    /// @throw std::domain_error if the divisor is 0.
    [[nodiscard]] static auto div(BigInt const &num, BigInt const &denom) -> std::pair<BigInt, BigInt>;

    /// @brief Raise the number to the specified power.
    ///
    /// @param power The power to raise the number to.
    /// @return The result of the exponentiation.
    ///
    /// @note Only works for non-negative powers.
    /// @note 0^0 returns 1.
    [[nodiscard]] auto pow(size_t power) const noexcept -> BigInt;

    friend std::formatter<BigInt>;
    friend auto operator""_bi(char const *) -> BigInt;

private:
    /// @brief Type used for each chunk of the number.
    using ChunkType = std::uint_fast32_t;

    /// @brief Type used to store the number.
    using DataType = std::vector<ChunkType>;

    static_assert(std::is_unsigned_v<ChunkType>, "ChunkType must be an unsigned integral type");
    static_assert(std::is_same_v<DataType::value_type, ChunkType>, "DataType must store ChunkType");

    /// @brief Sign of the number.
    bool negative{false};
    /// @brief Chunks of the number. Stored in little endian.
    DataType chunks;

    /// @brief Supported bases.
    enum class Base : std::uint_fast8_t
    {
        Binary = 2,
        Octal = 8,
        Decimal = 10,
        Hexadecimal = 16
    };

    /// @brief Number of bits in a chunk.
    static constexpr auto chunk_bits = sizeof(ChunkType) * 8;
    /// @brief Maximum value a chunk can store.
    static constexpr ChunkType chunk_max = std::numeric_limits<ChunkType>::max();

    /// @brief Get the number of bits in the number.
    [[nodiscard]] auto bit_count() const -> size_t;

    /// @brief Get the bit at the specified index.
    ///
    /// @param index The index of the bit to get. 0th bit is the least significant bit and the last bit is the most
    ///              significant bit.
    [[nodiscard]] auto get_bit_at(size_t index) const -> bool;

    /// @brief Check if the number is zero.
    [[nodiscard]] auto is_zero() const -> bool;
    /// @brief Remove leading zero chunks from the number.
    void remove_leading_zeroes();

    /// @brief Compare the magnitude of two numbers. Does not evaluate the sign.
    ///
    /// @param rhs The number to compare to.
    /// @return Ordering of the magnitude of the two numbers.
    [[nodiscard]] auto compare_magnitude(BigInt const &rhs) const noexcept -> std::strong_ordering;

    /// @brief Add the magnitude of lhs and rhs.
    ///
    /// @param rhs The number to add, must be smaller than or equal to lhs.
    /// @return The result of the addition.
    [[nodiscard]] auto add_magnitude(BigInt const &rhs) const noexcept -> BigInt;

    /// @brief Subtract the magnitude of rhs from lhs.
    ///
    /// @param rhs The number to subtract, must be smaller than or equal to lhs.
    /// @return The result of the subtraction.
    [[nodiscard]] auto subtract_magnitude(BigInt const &rhs) const noexcept -> BigInt;

    /// @brief Check if character is a valid digit in the given base.
    ///
    /// @param base The base to check the digit in.
    /// @param c The character to check.
    /// @return True if the character is a valid digit in the given base, false otherwise.
    ///
    /// @note Only works for bases 2, 8, 10, and 16.
    [[nodiscard]] static auto is_valid_digit(Base base, char c) -> bool;

    /// @brief Convert character to digit in the given base. The character must be a valid digit in the given base.
    ///
    /// @param base The base to convert the character to.
    /// @param c The character to convert.
    /// @return The digit represented by the character.
    ///
    /// @throws std::invalid_argument if the character is not a valid digit in the given base.
    /// @note Only works for bases 2, 8, 10, and 16.
    [[nodiscard]] static auto char_to_digit(Base base, char c) -> ChunkType;

    /// @brief Long divide string representation of a number by a divisor.
    ///
    /// @param num The number to divide, must be unsigned.
    /// @param[out] quotient Resulting quotient.
    /// @param base The base of the number.
    /// @param divisor The divisor.
    /// @return The remainder.
    ///
    /// @throws std::invalid_argument if num contains invalid digits for the given base.
    /// @note Only works for bases 2, 8, 10, and 16.
    static auto long_divide(std::string_view num, std::string &quotient, Base base, ChunkType divisor) -> ChunkType;

    /// @brief Convert string with power of two base to binary and store it in chunks.
    ///
    /// @param num The number to convert, must be unsigned.
    /// @param base The base of the number.
    ///
    /// @throws std::invalid_argument if num contains invalid digits for the given base.
    /// @note Only works for bases 2, 8, and 16.
    void power_of_two_base_to_binary(std::string_view num, Base base);

    /// @brief Convert decimal base to binary and store it in chunks.
    ///
    /// @param num The number to convert, must be unsigned.
    ///
    /// @throws std::invalid_argument if num contains invalid digits for the given base.
    void decimal_base_to_binary(std::string_view num);

    /// @brief Convert a base to binary and store it in chunks.
    ///
    /// @param num The number to convert, must be unsigned.
    /// @param base The base of the number.
    ///
    /// @throws std::invalid_argument if num contains invalid digits for the given base.
    /// @note Only works for bases 2, 8, 10, and 16.
    void base_to_binary(std::string_view num, Base base);

    /// @brief Format the number to a power of two base.
    ///
    /// @param base The base to format the number to.
    /// @param add_prefix Whether to add a base prefix to the formatted number (e.g. 0b for binary).
    /// @param capitalize Whether to capitalize the base prefix (if any) and the digits (for hexadecimal).
    /// @return The formatted number.
    ///
    /// @note Only works for bases 2, 8, and 16.
    [[nodiscard]] auto
    format_to_power_of_two_base(Base base, bool add_prefix = false, bool capitalize = false) const noexcept
      -> std::string;

    /// @brief Format the number to decimal.
    ///
    /// @return The formatted number.
    [[nodiscard]] auto format_to_decimal() const -> std::string;

    /// @brief Format the number to the specified base.
    ///
    /// @param base The base to format the number to.
    /// @param add_prefix Whether to add a base prefix to the formatted number (e.g. 0b for binary).
    /// @param capitalize Whether to capitalize the base prefix (if any) and the digits (for hexadecimal).
    /// @return The formatted number.
    ///
    /// @note Only works for bases 2, 8, 10, and 16.
    [[nodiscard]] auto format_to_base(Base base, bool add_prefix = false, bool capitalize = false) const -> std::string;
};
}  // namespace BI

auto operator<<(std::ostream &os, BI::BigInt const &num) -> std::ostream &;
auto operator""_bi(char const *) -> BI::BigInt;

template<>
struct std::formatter<BI::BigInt> : std::formatter<std::string>
{
    using BigInt = BI::BigInt;

    bool add_prefix = false;
    bool capitalize = false;
    BigInt::Base base{BigInt::Base::Decimal};

    constexpr auto parse(std::format_parse_context &ctx) -> decltype(ctx.begin())
    {
        auto const *it = ctx.begin();

        if (it == ctx.end() || *it == '}') {
            return it;
        }

        if (*it == '#') {
            add_prefix = true;
            std::advance(it, 1);
        }

        switch (*it) {
        case 'B':
            capitalize = true;
            [[fallthrough]];
        case 'b':
            base = BigInt::Base::Binary;
            break;
        case 'o':
            base = BigInt::Base::Octal;
            break;
        case 'd':
            base = BigInt::Base::Decimal;
            break;
        case 'X':
            capitalize = true;
            [[fallthrough]];
        case 'x':
            base = BigInt::Base::Hexadecimal;
            break;
        default:
            throw std::format_error("Invalid format specifier");
        }

        std::advance(it, 1);

        if (it != ctx.end() && *it != '}') {
            throw std::format_error("Invalid format specifier");
        }

        return it;
    }

    auto format(BigInt const &num, std::format_context &ctx) const
    {
        return std::format_to(ctx.out(), "{}", num.format_to_base(base, add_prefix, capitalize));
    }
};
