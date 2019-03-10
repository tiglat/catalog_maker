#pragma once
#include "IStringOperations.h"

class AnsiStringOperations : public IStringOperations<char>
{
public:
    AnsiStringOperations();
    ~AnsiStringOperations();
};

