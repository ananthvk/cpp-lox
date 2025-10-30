#include "compiler_fwd.hpp"
#include "gc.hpp"


// This stub should be used when compiling only the VM when the compiler source is not included

auto Compiler::mark_compiler_roots(GarbageCollector &gc, bool is_vm_live) -> void {}

auto Compiler::does_compiler_exist() -> bool { return false; }
