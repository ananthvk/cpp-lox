#pragma once
const int MAX_NUMBER_OF_LOCAL_VARIABLES = 2048;
const int MAX_STACK_EVALUATION_SIZE = 1024;
const int MAX_FRAME_SIZE = 256;
const int MAX_FUNCTION_PARAMETERS = 255;
const int DEFAULT_GC_HEAP_GROW_FACTOR = 2;
const int DEFAULT_GC_NEXT_COLLECTION = 1024 * 1024;

// If a list declared in the source file has greater than this number of values, the compiler will
// start emitting LIST_APPEND instructions to prevent stack overflow
// Note: This value should be < 256
const int LIST_BUILD_APPEND_THRESHOLD = 5; // 64;