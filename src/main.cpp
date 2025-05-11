#include <fmt/format.h>
#include "lox.hpp"
#include "logger.hpp"
#include <iostream>

int main() {
    fmt::println("Hello world: Sum is {}", add(3, 8));
    logger::info("Value = {}", 3);
}