#include <iostream>
#include <sys/socket.h>

#include "./test.h"

int main()
{
    const char* str = "Hello World !";

    test t = test();

    std::cout << str << "\n" << t.tt() << std::endl;

    return 0;
}