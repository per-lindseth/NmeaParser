#pragma once

#include "ErrorCodes.h"

struct Exception
{
    ErrorCode errorCode;
        const char* indication;

    Exception(ErrorCode e, const char* i = nullptr) :
        errorCode(e),
        indication(i)
    {
    }
};