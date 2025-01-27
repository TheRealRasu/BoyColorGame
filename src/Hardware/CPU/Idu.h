#pragma once

#include <cstdint>

class Idu
{
public:
    Idu();
    ~Idu() = default;

    template<typename T>
    void increaseValue(T& givenValue) const;

    template<typename T>
    void decreaseValue(T& givenValue) const;

    template<typename T>
    void assignValue(const T& valueSrc, T& valueDst) const;
};