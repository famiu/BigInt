#include <concepts>
#include <cstdint>
#include <string>
#include <vector>

class BigInt
{
public:
    using ChunkType = std::uint32_t;
    using DataType = std::vector<ChunkType>;

    constexpr BigInt();
    constexpr BigInt(BigInt const &rhs) = default;
    constexpr BigInt(BigInt &&rhs) = default;
    constexpr explicit BigInt(std::integral auto const &num);
    explicit BigInt(std::string_view num);
    constexpr auto operator=(BigInt const &rhs) noexcept -> BigInt & = default;
    constexpr auto operator=(BigInt &&rhs) noexcept -> BigInt & = default;
    ~BigInt() = default;

    constexpr auto operator+(BigInt const &rhs) const noexcept -> BigInt;
    constexpr auto operator-(BigInt const &rhs) const noexcept -> BigInt;
    constexpr auto operator*(BigInt const &rhs) const noexcept -> BigInt;
    constexpr auto operator/(BigInt const &rhs) const noexcept -> BigInt;
    constexpr auto operator%(BigInt const &rhs) const noexcept -> BigInt;
    constexpr auto operator-() const noexcept;

    constexpr auto operator&(BigInt const &rhs) const noexcept -> BigInt;
    constexpr auto operator|(BigInt const &rhs) const noexcept -> BigInt;
    constexpr auto operator^(BigInt const &rhs) const noexcept -> BigInt;
    constexpr auto operator~() const noexcept;

    constexpr auto operator<<(BigInt const &rhs) const noexcept -> BigInt;
    constexpr auto operator>>(BigInt const &rhs) const noexcept -> BigInt;

    constexpr auto operator+=(BigInt const &rhs) noexcept -> BigInt &;
    constexpr auto operator-=(BigInt const &rhs) noexcept -> BigInt &;
    constexpr auto operator*=(BigInt const &rhs) noexcept -> BigInt &;
    constexpr auto operator/=(BigInt const &rhs) noexcept -> BigInt &;
    constexpr auto operator%=(BigInt const &rhs) noexcept -> BigInt &;

    constexpr auto operator&=(BigInt const &rhs) noexcept -> BigInt &;
    constexpr auto operator|=(BigInt const &rhs) noexcept -> BigInt &;
    constexpr auto operator^=(BigInt const &rhs) noexcept -> BigInt &;

    constexpr auto operator<<=(BigInt const &rhs) noexcept -> BigInt &;
    constexpr auto operator>>=(BigInt const &rhs) noexcept -> BigInt &;

    constexpr auto operator<=>(BigInt const &rhs) const noexcept;

    explicit operator std::string() const;
    explicit operator std::integral auto() const;

    auto convert(std::string const &output) noexcept -> bool;
    auto convert(std::integral auto const &output) noexcept -> bool;

    friend auto operator""_bi(char const *) -> BigInt;

private:
    /// @brief Chunks of the number. Stored in little endian.
    DataType chunks;

    enum class Base : std::uint_fast8_t
    {
        Binary = 2,
        Octal = 8,
        Decimal = 10,
        Hexadecimal = 16
    };

    static auto is_valid_digit(Base base, char c) -> bool;
    static auto char_to_digit(Base base, char c) -> ChunkType;
    void base_to_binary(Base base, std::string_view num);
    void to_twos_complement();
    [[nodiscard]] auto to_base(Base base) const -> std::string;
};

auto operator""_bi(char const *) -> BigInt;
