#pragma once

#include <concepts>
#include <cstdint>
#include <deque>
#include <format>
#include <limits>
#include <stdexcept>
#include <string>

class BigInt
{
public:
    using ChunkType = std::uint_fast32_t;
    using DataType = std::deque<ChunkType>;

    BigInt();
    BigInt(BigInt const &rhs) = default;
    BigInt(BigInt &&rhs) noexcept = default;
    explicit BigInt(std::integral auto const &num);
    explicit BigInt(std::string_view num);
    auto operator=(BigInt const &rhs) noexcept -> BigInt & = default;
    auto operator=(BigInt &&rhs) noexcept -> BigInt & = default;
    ~BigInt() = default;

    auto operator+() const noexcept -> BigInt;
    auto operator-() const noexcept -> BigInt;

    auto operator+(BigInt const &rhs) const noexcept -> BigInt;
    auto operator-(BigInt const &rhs) const noexcept -> BigInt;
    auto operator*(BigInt const &rhs) const noexcept -> BigInt;
    auto operator/(BigInt const &rhs) const -> BigInt;
    auto operator%(BigInt const &rhs) const -> BigInt;

    auto operator&(BigInt const &rhs) const noexcept -> BigInt;
    auto operator|(BigInt const &rhs) const noexcept -> BigInt;
    auto operator^(BigInt const &rhs) const noexcept -> BigInt;
    auto operator~() const noexcept -> BigInt;

    auto operator<<(size_t rhs) const noexcept -> BigInt;
    auto operator>>(size_t rhs) const noexcept -> BigInt;

    auto operator+=(BigInt const &rhs) noexcept -> BigInt &;
    auto operator-=(BigInt const &rhs) noexcept -> BigInt &;
    auto operator*=(BigInt const &rhs) noexcept -> BigInt &;
    auto operator/=(BigInt const &rhs) noexcept -> BigInt &;
    auto operator%=(BigInt const &rhs) noexcept -> BigInt &;

    auto operator&=(BigInt const &rhs) noexcept -> BigInt &;
    auto operator|=(BigInt const &rhs) noexcept -> BigInt &;
    auto operator^=(BigInt const &rhs) noexcept -> BigInt &;

    auto operator<<=(size_t rhs) noexcept -> BigInt &;
    auto operator>>=(size_t rhs) noexcept -> BigInt &;

    auto operator++() noexcept -> BigInt &;
    auto operator--() noexcept -> BigInt &;
    auto operator++(int) noexcept -> BigInt;
    auto operator--(int) noexcept -> BigInt;

    auto operator<=>(BigInt const &rhs) const noexcept -> std::strong_ordering;
    auto operator==(BigInt const &rhs) const noexcept -> bool;

    template<std::integral T>
    auto operator<=>(T const &rhs) const noexcept -> std::strong_ordering
    {
        try {
            return static_cast<T>(*this) <=> rhs;
        } catch (std::overflow_error const &) {
            return negative ? std::strong_ordering::less : std::strong_ordering::greater;
        }
    }

    template<std::integral T>
    auto operator==(T const &rhs) const noexcept -> bool
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
            throw std::overflow_error(std::format("Number can't fit in unsigned type '{}'", typeid(T).name()));
        }
        // Signed types can store 1 less bit than their signed counterpart.
        if (num_bits > (sizeof(T) * 8) - static_cast<size_t>(is_signed)) {
            throw std::overflow_error(  // clang-format off
                    std::format("Number is too large to be converted to type '{}'", typeid(T).name())
            );  // clang-format on
        }

        UnsignedT result{};

        for (size_t i = 0; i < sizeof(T) * 8; i += chunk_bits) {
            result |= static_cast<T>(this->chunks[i / chunk_bits]) << i;
        }

        return negative ? -static_cast<T>(result) : static_cast<T>(result);
    }

    explicit operator std::string() const;

    [[nodiscard]] auto abs() const noexcept -> BigInt;

    auto convert(std::string const &output) noexcept -> bool;
    auto convert(std::integral auto const &output) noexcept -> bool;

    static auto div(BigInt const &num, BigInt const &denom) -> std::pair<BigInt, BigInt>;
    friend auto operator""_bi(char const *) -> BigInt;

private:
    /// Sign of the number.
    bool negative{false};
    /// @brief Chunks of the number. Stored in little endian.
    DataType chunks;

    enum class Base : std::uint_fast8_t
    {
        Binary = 2,
        Octal = 8,
        Decimal = 10,
        Hexadecimal = 16
    };

    constexpr static auto chunk_bits = sizeof(ChunkType) * 8;
    constexpr static ChunkType chunk_max = std::numeric_limits<ChunkType>::max();

    [[nodiscard]] auto bit_count() const -> size_t;
    [[nodiscard]] auto get_bit_at(size_t index) const -> bool;

    [[nodiscard]] auto is_zero() const -> bool;
    [[nodiscard]] auto is_negative() const -> bool;
    void remove_leading_zeroes();

    [[nodiscard]] auto compare_magnitude(BigInt const &rhs) const noexcept -> std::strong_ordering;

    static auto is_valid_digit(Base base, char c) -> bool;
    static auto char_to_digit(Base base, char c) -> ChunkType;
    static auto long_divide(std::string_view num, std::string &quotient, Base base, ChunkType divisor) -> ChunkType;
    void power_of_two_base_to_binary(std::string_view num, Base base);
    void decimal_base_to_binary(std::string_view num);
    void base_to_binary(std::string_view num, Base base);

    [[nodiscard]] auto to_power_of_two_base(Base base) const -> std::string;
    [[nodiscard]] auto to_decimal() const -> std::string;
    [[nodiscard]] auto to_base(Base base) const -> std::string;
};

auto operator""_bi(char const *) -> BigInt;

static_assert(std::is_unsigned_v<BigInt::ChunkType>, "ChunkType must be an unsigned integral type");
static_assert(std::is_same_v<BigInt::DataType::value_type, BigInt::ChunkType>, "DataType must store ChunkType");
