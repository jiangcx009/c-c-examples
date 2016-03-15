#include <iostream>

#include "test-extern-cc.h"

int foo(int a) {
    std::cout << "c++ foo() print: " << a << std::endl;
}
