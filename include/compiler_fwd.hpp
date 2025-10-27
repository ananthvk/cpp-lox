#pragma once

class GarbageCollector;

class Compiler
{
  public:
    static void mark_compiler_roots(GarbageCollector &, bool);
    static bool does_compiler_exist();
};