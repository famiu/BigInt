#pragma once

#include <limits>
#include <stdexcept>

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
