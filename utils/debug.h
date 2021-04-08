//
// Created by rory on 2021/3/5.
//

#ifndef JUDGERTOTEACHING_DEBUG_H
#define JUDGERTOTEACHING_DEBUG_H
#include <iostream>
using std::endl;
using std::cout;

#if DEBUG != 1
    #define DEBUG_PRINT(x)
#else
    #define DEBUG_PRINT(x) cout<<x<<endl;
#endif

#endif //JUDGERTOTEACHING_DEBUG_H
