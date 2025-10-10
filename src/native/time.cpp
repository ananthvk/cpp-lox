#include "value.hpp"
#include <chrono>

Value native_clock([[maybe_unused]] int arg_count, Value *args)
{
    using namespace std::chrono;
    double seconds = duration<double>(high_resolution_clock::now().time_since_epoch()).count();
    return Value{seconds};
}