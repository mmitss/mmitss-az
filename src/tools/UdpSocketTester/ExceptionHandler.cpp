#include <iostream>
#include "ExceptionHandler.h"

using std::cout;
using std::endl;

ExceptionHandler::ExceptionHandler(int exception)
{
    if (exception==ExSOCKET_BINDING_FAILURE)
    {
        cout << "hello" << endl;
        return (1);
    }
}