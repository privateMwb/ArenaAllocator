#pragma once

#include "Arena.h"
#include "ArenaScope.h"

#include <iostream>
#include <string>
#include <iomanip>

// terminal colors
constexpr const char* RESET  = "\033[0m";
constexpr const char* CYAN   = "\033[96m";
constexpr const char* GRAY   = "\033[37m";
constexpr const char* BLUE   = "\033[94m";

// prints a horizontal separator line
inline void borderLine() {
    std::cout << GRAY << std::string(70, '-') << RESET << "\n";
}

// prints the main title in blue
inline void mainTitle(std::string_view title) {
    std::cout << BLUE  << title << RESET << "\n";
}

// prints a section title in cyan
inline void setTitle(std::string_view title) {
    std::cout << CYAN << title << RESET << "\n";
}

// prints a formatted label-value pair
template<typename T>
inline void dataFormat(std::string_view left, T value) {
    std::cout << std::left
              << std::setw(30) << left << ":"
              << std::setw(15) << value
              << "\n";
}