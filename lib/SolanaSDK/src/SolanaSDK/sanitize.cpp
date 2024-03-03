#include "sanitize.h"

SanitizeError::SanitizeError(ErrorType error) : error(error) {}

const char *SanitizeError::what() const throw()
{
    switch (error)
    {
    case IndexOutOfBounds:
        return "index out of bounds";
    case ValueOutOfBounds:
        return "value out of bounds";
    case InvalidValue:
        return "invalid value";
    }
    return "";
}

template <typename T>
void Sanitize<T>::sanitize()
{
    // Default implementation does nothing
}

template <typename T>
Vec<T>::Vec(std::vector<T> vec) : vec(vec) {}

template <typename T>
void Vec<T>::sanitize()
{
    for (auto &x : vec)
    {
        x.sanitize();
    }
}
