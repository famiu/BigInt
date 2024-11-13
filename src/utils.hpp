#pragma once

#include <limits>
#include <source_location>
#include <stdexcept>
#include <utility>

namespace BI::detail
{
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

/// @brief Get the raw function name of a function with the type as a template parameter.
template<typename T>
constexpr auto type_to_string_raw() -> std::string_view
{
    return std::source_location::current().function_name();
}

/// @brief Raw function name of long double type, used to find the length of the prefix and suffix of the raw string.
constexpr std::string_view long_double_raw_string = type_to_string_raw<long double>();
constexpr std::string_view long_double_string = "long double";

template<std::size_t... Idxs>
constexpr auto substring_as_array(std::string_view str, std::index_sequence<Idxs...> /*unused*/)
{
    return std::array{str[Idxs]...};
}

template<typename T>
constexpr auto type_name_array()
{
    // Find prefix and suffix of every raw string using known type, by searching for the known type in the raw string.
    static constexpr auto prefix_len = long_double_raw_string.find(long_double_string);
    static constexpr auto suffix_len = long_double_raw_string.size() - prefix_len - long_double_string.size();

    constexpr std::string_view type_raw_string = type_to_string_raw<T>();
    constexpr auto start = prefix_len;
    constexpr auto end = type_raw_string.size() - suffix_len;

    static_assert(start < end);

    constexpr auto name = type_raw_string.substr(start, (end - start));

    // Convert the substring to a std::array so it can be used in a constexpr context.
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
}  // namespace BI::detail
