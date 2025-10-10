#include "vm.hpp"
#include "function.hpp"

void register_time(VM *vm);
void register_stdio(VM *vm);
void register_stdlib(VM *vm);
void register_math(VM *vm);

auto VM::register_native_functions() -> void
{
    register_time(this);
    register_stdio(this);
    register_stdlib(this);
    register_math(this);
}