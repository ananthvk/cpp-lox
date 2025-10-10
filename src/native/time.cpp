#include "value.hpp"
#include "vm.hpp"
#include <chrono>

auto native_clock(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    using namespace std::chrono;
    double seconds = duration<double>(high_resolution_clock::now().time_since_epoch()).count();
    return {Value{seconds}, true};
}

auto register_time(VM *vm) -> void { vm->define_native_function("clock", 0, native_clock); }