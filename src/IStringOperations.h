#pragma once

template <typename TChar> class IStringOperations
{
public:
    virtual bool IsNotNullOrEmpty(const TChar* pStr) = 0;
    virtual ~IStringOperations() {};
};

