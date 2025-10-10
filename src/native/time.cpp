#include "value.hpp"
#include "vm.hpp"
#include <chrono>

std::pair<Value, bool> native_clock([[maybe_unused]] VM *vm, [[maybe_unused]] int arg_count,
                                    Value *args)
{
    using namespace std::chrono;
    double seconds = duration<double>(high_resolution_clock::now().time_since_epoch()).count();
    return {Value{seconds}, true};
}

void register_time(VM *vm) { vm->define_native_function("clock", 0, native_clock); }