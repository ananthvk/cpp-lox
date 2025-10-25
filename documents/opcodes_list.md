## `RETURN`
**Stack:** `[value] -> []`

**Operands:** None

**Description:** Pops the top value from the stack and returns it from the current function. Exits the current call frame.


## `LOAD_CONSTANT`
**Stack:** `[] -> [value]`

**Operands:** 1 byte (constant index)

**Description:** Loads a constant from the constant pool at the given index and pushes it onto the stack.


## `LOAD_CONSTANT_LONG`
**Stack:** `[] -> [value]`

**Operands:** 2 bytes (16-bit constant index)

**Description:** Loads a constant from the constant pool at the given 16-bit index and pushes it onto the stack. Used for large constant pools.


## `NEGATE`
**Stack:** `[number] -> [number]`

**Operands:** None

**Description:** Negates a value on the stack in place.

## `ADD`
**Stack:** `[left, right] -> [result]`

**Operands:** None

**Description:** Pops two values from the stack, adds them together, and pushes the result. Supports numeric addition and string concatenation.

## `SUBTRACT`
**Stack:** `[left, right] -> [result]`

**Operands:** None

**Description:** Pops two numeric values from the stack, subtracts the top value from the second value, and pushes the result.

## `MULTIPLY`
**Stack:** `[left, right] -> [result]`

**Operands:** None

**Description:** Pops two numeric values from the stack, multiplies them, and pushes the result.

## `DIVIDE`
**Stack:** `[left, right] -> [result]`

**Operands:** None

**Description:** Pops two numeric values from the stack, divides the second value by the top value, and pushes the result.

## `NIL`
**Stack:** `[] -> [nil]`

**Operands:** None

**Description:** Pushes the nil value onto the stack.

## `UNINITIALIZED`
**Stack:** `[] -> [uninitialized]`

**Operands:** None

**Description:** Pushes an uninitialized marker value onto the stack (used to detect uninitialized variable access)

## `TRUE`
**Stack:** `[] -> [true]`

**Operands:** None

**Description:** Pushes the boolean value true onto the stack.

## `FALSE`
**Stack:** `[] -> [false]`

**Operands:** None

**Description:** Pushes the boolean value false onto the stack.

## `NOT`
**Stack:** `[value] -> [boolean]`

**Operands:** None

**Description:** Pops a value from the stack, applies logical NOT operation, and pushes the boolean result. It pushes the result of calling `is_falsey()` on the value. So true values become false, and false values become true.

## `EQUAL`
**Stack:** `[left, right] -> [boolean]`

**Operands:** None

**Description:** Pops two values from the stack, compares them for equality, and pushes the boolean result. TODO: Real numbers are compared by `==` which does not work correctly

## `GREATER`
**Stack:** `[left, right] -> [boolean]`

**Operands:** None

**Description:** Pops two values from the stack, checks if the second is greater than the top, and pushes the boolean result.

## `LESS`
**Stack:** `[left, right] -> [boolean]`

**Operands:** None

**Description:** Pops two values from the stack, checks if the second is less than the top, and pushes the boolean result.

## `PRINT`
**Stack:** `[value] -> []`

**Operands:** None

**Description:** Pops a value from the stack and prints it to output stream.

## `POP_TOP`
**Stack:** `[value] -> []`

**Operands:** None

**Description:** Pops and discards the top value from the stack.

## `DEFINE_GLOBAL`
**Stack:** `[value] -> []`

**Operands:** 2 byte (name index)

**Description:** Pops a value from the stack and defines a global variable with the name from the constant pool.

## `STORE_GLOBAL`
**Stack:** `[value] -> [value]`

**Operands:** 2 byte (name index)

**Description:** Loads a value from the top of the stack and stores it in an existing global variable. It differs from `DEFINE_GLOBAL` as it does not pop the value after storing it, so it can be used in other expressions.

## `LOAD_GLOBAL`
**Stack:** `[] -> [value]`

**Operands:** 2 byte (name index)

**Description:** Loads a global variable value by name from the constant pool and pushes it onto the stack.

## `STORE_LOCAL`
**Stack:** `[value] -> [value]`

**Operands:** 2 byte (slot index)

**Description:** Loads a value from the top of the stack and stores it in a local variable slot. It does not pop the value, since assignment is an expression.

## `LOAD_LOCAL`
**Stack:** `[] -> [value]`

**Operands:** 2 byte (slot index)

**Description:** Loads a local variable from the given slot and pushes it onto the stack.

## `STORE_UPVALUE`
**Stack:** `[value] -> [value]`

**Operands:** 2 byte (upvalue index)

**Description:** Loads a value from the top of the stack and stores it in an upvalue (captured variable).

## `LOAD_UPVALUE`
**Stack:** `[] -> [value]`

**Operands:** 2 byte (upvalue index)

**Description:** Loads an upvalue (captured variable) and pushes it onto the stack.

## `STORE_PROPERTY`
**Stack:** `[instance, value] -> [value]`

**Operands:** 2 byte (property name index)

**Description:** Pops value and instance from the stack, sets the property on the instance. And then pushes back the value onto the stack so that it can be used in other expressions.

## `LOAD_PROPERTY`
**Stack:** `[instance] -> [value]`

**Operands:** 2 byte (property name index)

**Description:** Pops an instance from the stack, gets the property value, and pushes it onto the stack.

## `LOAD_PROPERTY_SAFE`
**Stack:** `[instance] -> [value | nil]`

**Operands:** 2 byte (property name index)

**Description:** Safe property access that returns nil if property doesn't exist instead of throwing an error.

## `LOAD_SUPER`
**Stack:** `[instance] -> [method]`

**Operands:** 2 byte (method name index)

**Description:** Loads a method from the superclass, binds it to the instance and pushes it onto the stack.

## `LOAD_INDEX`
**Stack:** `[object, index] -> [value]`

**Operands:** None

**Description:** Pops index and object, performs indexing operation, and pushes the result.

## `STORE_INDEX`
**Stack:** `[object, index, value] -> [value]`

**Operands:** None

**Description:** Pops value, index, and object from the stack, stores value at the given index in the object. Pushes back the value at the end so that it can be used in other expressions.

## `CLOSE_UPVALUE`
**Stack:** `[...] -> [...]`

**Operands:** None

**Description:** Closes upvalues for local variables that are going out of scope.

## `JUMP_IF_FALSE`
**Stack:** `[condition] -> [condition]`

**Operands:** 2 bytes (jump offset)

**Description:** Peeks at the top value, jumps by offset if it's falsy, leaves the value on stack.

## `JUMP_IF_TRUE`
**Stack:** `[condition] -> [condition]`

**Operands:** 2 bytes (jump offset)

**Description:** Peeks at the top value, jumps by offset if it's truthy, leaves the value on stack.

## `POP_JUMP_IF_FALSE`
**Stack:** `[condition] -> []`

**Operands:** 2 bytes (jump offset)

**Description:** Pops the top value, jumps by offset if it's falsy.

## `JUMP_FORWARD`
**Stack:** `[...] -> [...]`

**Operands:** 2 bytes (jump offset)

**Description:** Unconditionally jumps forward by the given offset.

## `JUMP_BACKWARD`
**Stack:** `[...] -> [...]`

**Operands:** 2 bytes (jump offset)

**Description:** Unconditionally jumps backward by the given offset (for loops).

## `DUP_TOP`
**Stack:** `[value] -> [value, value]`

**Operands:** None

**Description:** Duplicates the top value on the stack.

## `CALL`
**Stack:** `[function, arg1, arg2, ...] -> [result]`

**Operands:** 1 byte (argument count)

**Description:** Calls a function with the given number of arguments, replacing them and the function with the result.

## `CLASS`
**Stack:** `[] -> [class]`

**Operands:** 2 byte (class name index)

**Description:** Creates a new class with the given name and pushes it onto the stack.

## `INVOKE`
**Stack:** `[instance, arg1, arg2, ...] -> [result]`

**Operands:** 3 bytes (method name index (2), argument count(1))

**Description:** Invokes a method on an instance with the given arguments.

## `SUPER_INVOKE`
**Stack:** `[instance, arg1, arg2, ...] -> [result]`

**Operands:** 3 bytes (method name index (2), argument count(1))

**Description:** Invokes a method from the superclass on an instance.

## `INHERIT`
**Stack:** `[superclass, subclass] -> [superclass]`

**Operands:** None

**Description:** Peeks superclass and pops subclass, copies methods from superclass to subclass.

## `CLOSURE`
**Stack:** `[] -> [closure]`

**Operands:** 2 bytes (function index) + variable bytes for upvalue data

**Description:** Creates a closure from a function and pushes it onto the stack. For each upvalue, reads 1 byte (is_local flag) + 2 bytes (index). If is_local is true, captures a local variable as an upvalue, otherwise, references an upvalue from the current function. This is a variable length instruction.

## `ZERO`
**Stack:** `[] -> [0]`

**Operands:** None

**Description:** Pushes the numeric value 0 onto the stack.

## `MINUS_ONE`
**Stack:** `[] -> [-1]`

**Operands:** None

**Description:** Pushes the numeric value -1 onto the stack.

## `ONE`
**Stack:** `[] -> [1]`

**Operands:** None

**Description:** Pushes the numeric value 1 onto the stack.

## `METHOD`
**Stack:** `[class, closure] -> [class]`

**Operands:** 2 byte (method name index)

**Description:** Peeks closure (for the method) and class, binds the closure to the class. Pops the closure from the top of the stack.

## `LIST`
**Stack:** `[item1, item2, ...] -> [list]`

**Operands:** 1 byte (item count)

**Description:** Pops the specified number of items and creates a list containing them.

## `LIST_APPEND`
**Stack:** `[list, item] -> [list]`

**Operands:** None

**Description:** Pops an item and list, appends the item to the list, pushes the list back.
