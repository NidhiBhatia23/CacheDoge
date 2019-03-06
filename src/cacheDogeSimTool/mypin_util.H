/*
 * Created on Mon Nov 19 2018
 * Author: Artur Balanuta & Nidhi Bhatia
 * Copyright (c) 2018 Carnegie Mellon University
 */

#ifndef MYPIN_UTIL_H
#define MYPIN_UTIL_H

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#define KILO 1024
#define MEGA (KILO*KILO)
#define GIGA (KILO*MEGA)

GLOBALFUN std::string StringInt(UINT64 val, UINT32 width=0, CHAR padding=' ')
{
    std::ostringstream ostr;
    ostr.setf(std::ios::fixed,std::ios::floatfield);
    ostr.fill(padding);
    ostr << std::setw(width) << val;
    return ostr.str();
}

GLOBALFUN std::string StringHex(UINT64 val, UINT32 width=0, CHAR padding=' ')
{
    std::ostringstream ostr;
    ostr.setf(std::ios::fixed,std::ios::floatfield);
    ostr.fill(padding);
    ostr << std::setw(width) << std::hex << "0x" << val;
    return ostr.str();
}

GLOBALFUN std::string StringString(std::string val, UINT32 width=0, CHAR padding=' ')
{
    std::ostringstream ostr;
    ostr.setf(std::ios::fixed,std::ios::floatfield);
    ostr.fill(padding);
    ostr << std::setw(width) << val;
    return ostr.str();
}

#endif // MYPIN_UTIL_H
