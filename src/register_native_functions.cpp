#include "vm.hpp"
#include "function.hpp"

auto register_time(VM *vm) -> void;
auto register_stdio(VM *vm) -> void;
auto register_stdlib(VM *vm) -> void;
auto register_math(VM *vm) -> void;
auto register_string(VM *vm) -> void;
auto register_test(VM *vm) -> void;
auto register_memory(VM *vm) -> void;

auto VM::register_native_functions() -> void
{
    register_time(this);
    register_stdio(this);
    register_stdlib(this);
    register_math(this);
    register_string(this);
    register_test(this);
    register_memory(this);
}