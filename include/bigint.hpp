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
    explicit BigInt(std::string const &num);
    constexpr auto operator=(BigInt const &rhs) noexcept -> BigInt & = default;
    constexpr auto operator=(BigInt &&rhs) noexcept -> BigInt & = default;
    ~BigInt() = default;

    constexpr auto operator+(BigInt const &rhs) const noexcept;
    constexpr auto operator-(BigInt const &rhs) const noexcept;
    constexpr auto operator*(BigInt const &rhs) const noexcept;
    constexpr auto operator/(BigInt const &rhs) const noexcept;
    constexpr auto operator%(BigInt const &rhs) const noexcept;
    constexpr auto operator-() const noexcept;

    constexpr auto operator&(BigInt const &rhs) const noexcept;
    constexpr auto operator|(BigInt const &rhs) const noexcept;
    constexpr auto operator^(BigInt const &rhs) const noexcept;
    constexpr auto operator~() const noexcept;

    constexpr auto operator<<(BigInt const &rhs) const noexcept;
    constexpr auto operator>>(BigInt const &rhs) const noexcept;

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
};

auto operator""_bi(char const *) -> BigInt;
