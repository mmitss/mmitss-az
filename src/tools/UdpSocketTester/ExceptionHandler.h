#pragma once

const int ExSOCKET_CREATION_FAILURE = 101;
const int ExSOCKET_BINDING_FAILURE = 102;
const int ExSOCKET_INVALID_PORTNO = 103;

class ExceptionHandler
{
    public:
        ExceptionHandler(int exception);
};