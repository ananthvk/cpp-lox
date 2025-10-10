#include "value.hpp"
#include "vm.hpp"
#include <math.h>
#include <random>

// Generates a random number in range [m, n)
auto static generate_random_int(int64_t m, int64_t n) -> int64_t
{
    static std::random_device device;
    static std::mt19937 gen(device());
    static std::uniform_int_distribution<int64_t> dist(m, n);
    return dist(gen);
}

// Generate random number in range [0, 1]
auto static generate_random_double() -> double
{
    static std::random_device device;
    static std::mt19937 gen(device());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}

auto native_math_sqrt(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
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

auto native_math_rand(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    return {Value{generate_random_double()}, true};
}

auto native_math_randint(VM *vm, int arg_count, Value *values) -> std::pair<Value, bool>
{
    auto m = values[1];
    auto n = values[2];
    if (!m.is_integer())
    {
        vm->report_error("invalid argument type to call randint(m, n), m must be an integer");
        return {Value{}, false};
    }
    if (!n.is_integer())
    {
        vm->report_error("invalid argument type to call randint(m, n), n must be an integer");
        return {Value{}, false};
    }
    return {Value{generate_random_int(m.as_integer(), n.as_integer())}, true};
}

auto register_math(VM *vm) -> void
{
    srand(static_cast<unsigned>(time(NULL)));
    vm->define_native_function("sqrt", 1, native_math_sqrt);
    vm->define_native_function("rand", 0, native_math_rand);
    vm->define_native_function("randint", 2, native_math_randint);
}