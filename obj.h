//
// Created by win on 2024/6/12.
//

#ifndef BIND_OBJ_H
#define BIND_OBJ_H

#include <iostream>

class Demo {
public:
    Demo() { std::cout << "Demo()" << std::endl; }

    ~Demo() { std::cout << "~Demo()" << std::endl; }

    static int say();
};

extern "C" Demo* constructDemo();
extern "C" void destroyDemo(Demo* demo);

#endif //BIND_OBJ_H
