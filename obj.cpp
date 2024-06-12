//
// Created by win on 2024/6/12.
//

#include "obj.h"

int Demo::say()
{
    return 1;
}

extern "C" Demo* constructDemo()
{
    return new Demo;
}

extern "C" void destroyDemo(Demo* demo)
{
    delete demo;
}
