#pragma once
#include "config.hpp"

struct VMOpts
{
    /**
     * Max number of entries allowed in the VM's stack, this stack is used to execute bytecode
     * in the VM. It is set to a default of 1024
     */
    int value_stack_max = MAX_STACK_EVALUATION_SIZE;

    int frames_max = MAX_FRAME_SIZE;

    bool debug_trace_execution = false;

    bool debug_trace_value_stack = false;

    /**
     * Waits for input before executing the next instruction
     */
    bool debug_step_mode_enabled = false;

    // If this is set to true, triggers a garbage collection everytime a new object is created
    bool debug_stress_gc = false;

    // If this is set to true, enables logging of gc during collection
    bool debug_log_gc = false;

    int gc_heap_grow_factor = DEFAULT_GC_HEAP_GROW_FACTOR;

    int gc_next_collection = DEFAULT_GC_NEXT_COLLECTION;

    // Display GC stats when the program exits
    bool display_mem_stats = false;
};
