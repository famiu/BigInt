#pragma once

#include <limits>
#include <stdexcept>
#include <utility>

template<typename T>
static constexpr auto to_unsigned(T const &num) -> std::make_unsigned_t<T>
{
    if constexpr (std::is_unsigned_v<T>) {
        return num;
    } else {
        if (num < 0) {
            throw std::underflow_error("Number is negative and cannot be converted to an unsigned type");
        }
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

template<std::size_t... Idxs>
constexpr auto substring_as_array(std::string_view str, std::index_sequence<Idxs...> /*unused*/)
{
    return std::array{str[Idxs]...};
}

template<typename T>
constexpr auto type_name_array()
{
#if defined(__clang__)
    constexpr auto prefix = std::string_view{"[T = "};
    constexpr auto suffix = std::string_view{"]"};
    constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
    constexpr auto prefix = std::string_view{"with T = "};
    constexpr auto suffix = std::string_view{"]"};
    constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
    constexpr auto prefix = std::string_view{"type_name_array<"};
    constexpr auto suffix = std::string_view{">(void)"};
    constexpr auto function = std::string_view{__FUNCSIG__};
#else
#   error Unsupported compiler
#endif

    constexpr auto start = function.find(prefix) + prefix.size();
    constexpr auto end = function.rfind(suffix);

    static_assert(start < end);

    constexpr auto name = function.substr(start, (end - start));

    return substring_as_array(name, std::make_index_sequence<name.size()>{});
};

template<typename T>
struct type_name_holder
{
    static constexpr auto value = type_name_array<T>();
};

/// @brief Get the name of a type as a string.
///
/// @tparam T The type to get the name of.
/// @return The name of the type.
///
/// @see https://rodusek.com/posts/2021/03/09/getting-an-unmangled-type-name-at-compile-time/
template<typename T>
constexpr auto type_name() -> std::string_view
{
    constexpr auto &value = type_name_holder<T>::value;
    return std::string_view(value.data(), value.size());
}
