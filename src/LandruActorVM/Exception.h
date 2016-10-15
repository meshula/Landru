//
//  Exception.h
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//

#pragma once

#include <exception>
#include <string>
#include <sstream>

namespace Landru {
    class Exception : public std::exception {
    public:
        Exception() {}
        Exception(const char* s) : s(s) {}
        Exception(const std::string& s) : s(s) {}
        Exception(const std::stringstream& str) : s(str.str()) {}
        virtual const char* what() const throw() { return s.c_str(); }
        std::string s;
    };
    #define VM_RAISE(text) do { std::stringstream s; s << "Runtime error: "; s << text; throw Exception(s); } while (0)
}
