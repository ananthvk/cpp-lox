#pragma once
#include "chunk.hpp"
#include "context.hpp"
#include "debug_vm.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <fmt/color.h>
#include <fmt/format.h>
#include <string>


auto print_tokens(const Lexer &lexer) -> void;