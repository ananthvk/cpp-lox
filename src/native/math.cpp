#include "value.hpp"
#include "vm.hpp"
#include <math.h>

std::pair<Value, bool> native_math_sqrt(VM *vm, int arg_count, Value *values)
{
    Value val = values[1];
    if (!val.is_number())
    {
        vm->report_error("invalid argument type to call sqrt(), must be a number");
        return {Value{}, false};
    }
    double value = val.coerce_real();
    return {Value{sqrt(value)}, true};
}

void register_math(VM *vm) { vm->define_native_function("sqrt", 1, native_math_sqrt); }