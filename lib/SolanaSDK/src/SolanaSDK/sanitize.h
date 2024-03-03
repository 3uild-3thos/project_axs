#ifndef SANITIZE_H
#define SANITIZE_H

#include <vector>

enum ErrorType
{
    IndexOutOfBounds,
    ValueOutOfBounds,
    InvalidValue
};

class SanitizeError
{
public:
    SanitizeError(ErrorType error);
    const char *what() const throw();

private:
    ErrorType error;
};

template <typename T>
class Sanitize
{
public:
    virtual void sanitize();
};

template <typename T>
class Vec : public Sanitize<T>
{
public:
    Vec(std::vector<T> vec);
    void sanitize() override;

private:
    std::vector<T> vec;
};

#endif // SANITIZE_H
