#pragma once

#include "windows.h"

#include "IStringOperations.h"

class WideStringOperations : public IStringOperations<WCHAR>
{
public:
    WideStringOperations();
    ~WideStringOperations();
};

