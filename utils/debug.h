//
// Created by rory on 2021/3/5.
//

#ifndef JUDGERTOTEACHING_DEBUG_H
#define JUDGERTOTEACHING_DEBUG_H
#include <iostream>
using std::cout;
using std::endl;

#ifdef DEBUG
    DEBUGPRINT(x) cout<<x<<endl;
else
    DEBUGPRINT(x)
#endif

#endif //JUDGERTOTEACHING_DEBUG_H
