import subprocess


def test_map_get_element_string_key(run_lox):
    assert run_lox('var x = {"name": "John", "age": 30}; echo x["name"];') == "John"


def test_map_get_element_numeric_key(run_lox):
    assert run_lox('var x = {1: "one", 2: "two"}; echo x[1];') == "one"


def test_map_set_element_new_key(run_lox):
    assert run_lox('var x = {"a": 1}; x["b"] = 2; echo x["b"];') == "2"


def test_map_set_element_existing_key(run_lox):
    assert run_lox('var x = {"a": 1}; x["a"] = 42; echo x["a"];') == "42"


def test_map_nested_maps(run_lox):
    assert (
        run_lox('var x = {"outer": {"inner": "value"}}; echo x["outer"]["inner"];')
        == "value"
    )


def test_map_with_list_values(run_lox):
    assert run_lox('var x = {"numbers": [1, 2, 3]}; echo x["numbers"][1];') == "2"


def test_map_assignment_reference(run_lox):
    assert (
        run_lox('var x = {"count": 5}; var y = x; y["count"] = 10; echo x["count"];')
        == "10"
    )


def test_map_mixed_key_types(run_lox):
    assert (
        run_lox('var x = {"str": "hello", 1: "number", true: "boolean"}; echo x[true];')
        == "boolean"
    )


def test_map_with_functions_as_values(run_lox):
    assert (
        run_lox(
            """
            fun greet(name) { return "Hello, " + name + "!"; }
            var actions = {"greeting": greet};
            echo actions["greeting"]("World");
        """
        )
        == "Hello, World!"
    )


def test_map_function_returning_map(run_lox):
    assert (
        run_lox(
            """
            fun createUser(name, age) {
                return {"name": name, "age": age, "isActive": true};
            }
            var user = createUser("Alice", 25);
            echo user["name"] + " is " + to_string(user["age"]) + " years old";
        """
        )
        == "Alice is 25 years old"
    )


def test_map_passed_to_function(run_lox):
    assert (
        run_lox(
            """
            fun processUser(userMap) {
                userMap["processed"] = true;
                return userMap["name"] + " processed";
            }
            var user = {"name": "Bob", "age": 30};
            var result = processUser(user);
            echo result + ", processed: " + to_string(user["processed"]);
        """
        )
        == "Bob processed, processed: true"
    )


def test_map_iteration_with_function(run_lox):
    assert (
        run_lox(
            """
            fun sumValues(map) {
                var total = 0;
                // Assuming some iteration mechanism exists
                total = map["a"] + map["b"] + map["c"];
                return total;
            }
            var numbers = {"a": 10, "b": 20, "c": 30};
            echo sumValues(numbers);
        """
        )
        == "60"
    )


def test_map_complex_nesting_with_functions(run_lox):
    assert (
        run_lox(
            """
            fun createNestedStructure() {
                return {
                    "users": {
                        "admin": {"name": "Admin", "permissions": ["read", "write"]},
                        "guest": {"name": "Guest", "permissions": ["read"]}
                    },
                    "settings": {"theme": "dark", "language": "en"}
                };
            }
            var app = createNestedStructure();
            echo app["users"]["admin"]["name"] + " has " + app["settings"]["theme"] + " theme";
        """
        )
        == "Admin has dark theme"
    )


def test_map_with_closure_functions(run_lox):
    assert (
        run_lox(
            """
            fun createCounter(initial) {
                var count = initial;
                fun increment() {
                    count = count + 1;
                    return count;
                }
                return {"value": count, "increment": increment};
            }
            var counter = createCounter(5);
            counter["increment"]();
            counter["increment"]();
            echo counter["increment"]();
        """
        )
        == "8"
    )


def test_map_dynamic_key_generation(run_lox):
    assert (
        run_lox(
            """
            var data = {};
            var prefix = "item_";
            for (var i = 1; i <= 3; i = i + 1) {
                var key = prefix + to_string(i);
                data[key] = i * 10;
            }
            echo to_string(data["item_1"]) + ", " + to_string(data["item_2"]) + ", " + to_string(data["item_3"]);
        """
        )
        == "10, 20, 30"
    )


def test_map_method_like_behavior(run_lox):
    assert (
        run_lox(
            """
            fun addMethod(obj, x) { obj["value"] = obj["value"] + x; return obj; }
            fun multiplyMethod(obj, x) { obj["value"] = obj["value"] * x; return obj; }
            var calculator = {
                "value": 0,
                "add": addMethod,
                "multiply": multiplyMethod
            };
            calculator["add"](calculator, 5);
            calculator["multiply"](calculator, 3);
            echo calculator["value"];
        """
        )
        == "15"
    )


def test_map_list_as_key_error_2(run_lox):
    try:
        run_lox('var x = {}; var key = [1, 2, 3]; x[key] = "value";')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_map_as_key_error_2(run_lox):
    try:
        run_lox('var x = {}; var key = {"nested": "map"}; x[key] = "value";')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_empty_initialization_and_size(run_lox):
    assert (
        run_lox(
            'var x = {}; x["first"] = 1; x["second"] = 2; echo to_string(x["first"]) + "," + to_string(x["second"]);'
        )
        == "1,2"
    )


def test_map_overwrite_with_different_types(run_lox):
    assert (
        run_lox(
            'var x = {"key": "string"}; x["key"] = 42; x["key"] = true; echo to_string(x["key"]);'
        )
        == "true"
    )


def test_map_nil_value_handling(run_lox):
    assert (
        run_lox(
            'var x = {"exists": "value", "missing": nil}; echo x["exists"] + "," + to_string(x["missing"]);'
        )
        == "value,nil"
    )


def test_map_conditional_access_pattern(run_lox):
    assert (
        run_lox(
            """
            var config = {"debug": true, "maxUsers": 100};
            var message = "";
            if (config["debug"]) {
                message = "Debug mode: " + to_string(config["maxUsers"]) + " max users";
            } else {
                message = "Production mode";
            }
            echo message;
        """
        )
        == "Debug mode: 100 max users"
    )


def test_map_closure_factory_with_state(run_lox):
    assert (
        run_lox(
            """
            fun createStateMachine() {
                var state = "idle";
                
                fun idleTransition() { state = "running"; return state; }
                fun runningTransition() { state = "stopped"; return state; }
                fun stoppedTransition() { state = "idle"; return state; }
                
                var transitions = {
                    "idle": idleTransition,
                    "running": runningTransition,
                    "stopped": stoppedTransition
                };
                
                fun getState() { return state; }
                fun transition() { return transitions[state](); }
                
                return {
                    "getState": getState,
                    "transition": transition
                };
            }
            var machine = createStateMachine();
            machine["transition"]();
            machine["transition"]();
            echo machine["getState"]();
        """
        )
        == "stopped"
    )


def test_map_nested_closures_with_shared_state(run_lox):
    assert (
        run_lox(
            """
            fun createBankAccount(initialBalance) {
                var balance = initialBalance;
                
                fun deposit(amount) {
                    balance = balance + amount;
                    return balance;
                }
                
                fun withdraw(amount) {
                    if (balance >= amount) {
                        balance = balance - amount;
                        return balance;
                    }
                    return balance;
                }
                
                fun getBalance() { return balance; }
                
                fun doubleDeposit(amount) {
                    balance = balance + (amount * 2);
                    return balance;
                }
                
                fun getPrivateBalance() { return balance; }
                
                return {
                    "deposit": deposit,
                    "withdraw": withdraw,
                    "getBalance": getBalance,
                    "operations": {
                        "doubleDeposit": doubleDeposit,
                        "getPrivateBalance": getPrivateBalance
                    }
                };
            }
            var account = createBankAccount(100);
            account["deposit"](50);
            account["operations"]["doubleDeposit"](25);
            echo account["operations"]["getPrivateBalance"]();
        """
        )
        == "200"
    )


def test_map_closure_chain_with_decorators(run_lox):
    assert (
        run_lox(
            """
            fun createLogger(prefix) {
                fun logger(message) {
                    return prefix + ": " + message;
                }
                return logger;
            }
            
            fun createTimestamped(baseLogger) {
                var counter = 0;
                fun timestamped(message) {
                    counter = counter + 1;
                    return "[" + to_string(counter) + "] " + baseLogger(message);
                }
                return timestamped;
            }
            
            var loggers = {
                "error": createTimestamped(createLogger("ERROR")),
                "info": createTimestamped(createLogger("INFO")),
                "debug": createTimestamped(createLogger("DEBUG"))
            };
            
            loggers["error"]("Something failed");
            loggers["info"]("Process started");
            echo loggers["error"]("Critical error");
        """
        )
        == "[2] ERROR: Critical error"
    )


def test_map_closure_event_system_with_callbacks(run_lox):
    assert (
        run_lox(
            """
            fun createEventManager() {
                var listeners = {};
                
                fun on(event, callback) {
                    listeners[event] = callback;
                }
                
                fun emit(event, data) {
                    if (listeners[event]) {
                        return listeners[event](data);
                    }
                    return "No listener";
                }
                
                fun createHandler(handlerName) {
                    var callCount = 0;
                    fun handler(data) {
                        callCount = callCount + 1;
                        return handlerName + "(" + to_string(callCount) + "): " + data;
                    }
                    return handler;
                }
                
                return {
                    "on": on,
                    "emit": emit,
                    "createHandler": createHandler
                };
            }
            
            var events = createEventManager();
            var userHandler = events["createHandler"]("UserHandler");
            var systemHandler = events["createHandler"]("SystemHandler");
            
            events["on"]("user", userHandler);
            events["on"]("system", systemHandler);
            
            events["emit"]("user", "login");
            events["emit"]("system", "startup");
            echo events["emit"]("user", "logout");
        """
        )
        == "UserHandler(2): logout"
    )


def test_map_complex_closure_composition_with_currying(run_lox):
    assert (
        run_lox(
            """
            fun createMathOperations() {
                var operations = {};
                
                fun addOp(x, y) { return x + y; }
                fun multiplyOp(x, y) { return x * y; }
                
                fun curry(operation) {
                    fun curryFirst(a) {
                        fun currySecond(b) {
                            return operation(a, b);
                        }
                        return currySecond;
                    }
                    return curryFirst;
                }
                
                operations["add"] = curry(addOp);
                operations["multiply"] = curry(multiplyOp);
                
                fun createChain(initial) {
                    var value = initial;
                    
                    fun add(x) {
                        var adder = operations["add"](value);
                        value = adder(x);
                        return value;
                    }
                    
                    fun multiply(x) {
                        var multiplier = operations["multiply"](value);
                        value = multiplier(x);
                        return value;
                    }
                    
                    fun getValue() { return value; }
                    
                    return {
                        "add": add,
                        "multiply": multiply,
                        "getValue": getValue
                    };
                }
                
                operations["createChain"] = createChain;
                return operations;
            }
            
            var math = createMathOperations();
            var chain = math["createChain"](5);
            chain["add"](3);
            chain["multiply"](2);
            chain["add"](4);
            echo chain["getValue"]();
        """
        )
        == "20"
    )


def test_map_len_function_empty(run_lox):
    assert run_lox("var x = {}; echo len(x);") == "0"


def test_map_len_function_with_elements(run_lox):
    assert run_lox('var x = {"a": 1, "b": 2, "c": 3}; echo len(x);') == "3"


def test_map_len_function_after_modifications(run_lox):
    assert run_lox('var x = {"a": 1}; x["b"] = 2; echo len(x);') == "2"


def test_map_native_constructor_function(run_lox):
    assert run_lox("var x = map(); echo len(x);") == "0"


def test_map_native_constructor_with_assignment(run_lox):
    assert run_lox('var x = map(); x["key"] = "value"; echo x["key"];') == "value"


def test_map_append_function_new_key(run_lox):
    assert run_lox('var x = {}; append(x, "name", "John"); echo x["name"];') == "John"


def test_map_append_function_existing_key(run_lox):
    assert run_lox('var x = {"age": 20}; append(x, "age", 30); echo x["age"];') == "30"


def test_map_append_function_multiple_keys(run_lox):
    assert (
        run_lox(
            'var x = {}; append(x, "a", 1); append(x, "b", 2); echo to_string(x["a"]) + "," + to_string(x["b"]);'
        )
        == "1,2"
    )


def test_map_delete_function_existing_key(run_lox):
    assert run_lox('var x = {"a": 1, "b": 2}; echo delete(x, "a");') == "true"


def test_map_delete_function_nonexistent_key(run_lox):
    assert run_lox('var x = {"a": 1}; echo delete(x, "missing");') == "false"


def test_map_delete_function_removes_key(run_lox):
    assert run_lox('var x = {"a": 1, "b": 2}; delete(x, "a"); echo len(x);') == "1"


def test_map_keys_function_empty_map(run_lox):
    assert run_lox("var x = {}; var k = keys(x); echo len(k);") == "0"


def test_map_keys_function_with_elements(run_lox):
    assert run_lox('var x = {"a": 1, "b": 2}; var k = keys(x); echo len(k);') == "2"


def test_map_keys_function_iteration(run_lox):
    assert (
        run_lox(
            """
            var x = {"name": "John", "age": 30, "city": "NYC"};
            var k = keys(x);
            var foundName = false;
            var foundAge = false;
            var foundCity = false;
            for (var i = 0; i < len(k); i = i + 1) {
                if (k[i] == "name") foundName = true;
                if (k[i] == "age") foundAge = true;
                if (k[i] == "city") foundCity = true;
            }
            echo to_string(foundName) + "," + to_string(foundAge) + "," + to_string(foundCity);
        """
        )
        == "true,true,true"
    )


def test_map_values_function_empty_map(run_lox):
    assert run_lox("var x = {}; var v = values(x); echo len(v);") == "0"


def test_map_values_function_with_elements(run_lox):
    assert run_lox('var x = {"a": 10, "b": 20}; var v = values(x); echo len(v);') == "2"


def test_map_values_function_sum_iteration(run_lox):
    assert (
        run_lox(
            """
            var x = {"a": 10, "b": 20, "c": 30};
            var v = values(x);
            var sum = 0;
            for (var i = 0; i < len(v); i = i + 1) {
                sum = sum + v[i];
            }
            echo sum;
        """
        )
        == "60"
    )


def test_map_has_function_existing_key(run_lox):
    assert run_lox('var x = {"name": "John"}; echo has(x, "name");') == "true"


def test_map_has_function_nonexistent_key(run_lox):
    assert run_lox('var x = {"name": "John"}; echo has(x, "age");') == "false"


def test_map_has_function_after_delete(run_lox):
    assert (
        run_lox('var x = {"a": 1, "b": 2}; delete(x, "a"); echo has(x, "a");')
        == "false"
    )


def test_map_clear_function_removes_all(run_lox):
    assert run_lox('var x = {"a": 1, "b": 2, "c": 3}; clear(x); echo len(x);') == "0"


def test_map_clear_function_empty_map(run_lox):
    assert run_lox("var x = {}; clear(x); echo len(x);") == "0"


def test_map_clear_function_keys_empty(run_lox):
    assert (
        run_lox('var x = {"a": 1, "b": 2}; clear(x); var k = keys(x); echo len(k);')
        == "0"
    )


def test_map_get_function_existing_key(run_lox):
    assert (
        run_lox('var x = {"name": "Alice"}; echo get(x, "name", "default");') == "Alice"
    )


def test_map_get_function_nonexistent_key_with_default(run_lox):
    assert (
        run_lox('var x = {"name": "Alice"}; echo get(x, "age", "unknown");')
        == "unknown"
    )


def test_map_get_function_nonexistent_key_no_default(run_lox):
    assert run_lox('var x = {"name": "Alice"}; echo get(x, "age");') == "nil"


def test_map_get_function_nil_value_vs_missing_key(run_lox):
    assert (
        run_lox(
            'var x = {"exists": nil}; echo to_string(get(x, "exists", "default")) + "," + get(x, "missing", "default");'
        )
        == "nil,default"
    )


def test_map_combined_operations_workflow(run_lox):
    assert (
        run_lox(
            """
            var inventory = map();
            append(inventory, "apples", 5);
            append(inventory, "bananas", 3);
            append(inventory, "oranges", 8);
            
            var total = 0;
            var items = keys(inventory);
            for (var i = 0; i < len(items); i = i + 1) {
                total = total + get(inventory, items[i], 0);
            }
            
            delete(inventory, "bananas");
            var remaining = len(inventory);
            
            echo to_string(total) + "," + to_string(remaining);
        """
        )
        == "16,2"
    )


def test_map_keys_values_correspondence(run_lox):
    assert (
        run_lox(
            """
            var data = {"x": 10, "y": 20, "z": 30};
            var k = keys(data);
            var v = values(data);
            var sum = 0;
            
            for (var i = 0; i < len(k); i = i + 1) {
                sum = sum + data[k[i]];
            }
            
            echo to_string(sum) + "," + to_string(len(v));
        """
        )
        == "60,3"
    )


def test_map_object_as_key_same_reference(run_lox):
    assert (
        run_lox(
            """
            class Person {}
            var p = Person();
            var registry = map();
            append(registry, p, "John");
            echo get(registry, p, "unknown");
        """
        )
        == "John"
    )


def test_map_object_as_key_different_instances(run_lox):
    assert (
        run_lox(
            """
            class Person {}
            var p1 = Person();
            var p2 = Person();
            var registry = map();
            append(registry, p1, "John");
            echo get(registry, p2, "unknown");
        """
        )
        == "unknown"
    )


def test_map_list_as_key_error(run_lox):
    try:
        run_lox('var x = map(); var key = [1, 2]; append(x, key, "value");')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_map_as_key_error(run_lox):
    try:
        run_lox('var x = map(); var key = {"nested": true}; append(x, key, "value");')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_delete_from_empty_map(run_lox):
    assert run_lox('var x = map(); echo delete(x, "anything");') == "false"


def test_map_has_with_nil_value(run_lox):
    assert run_lox('var x = {"key": nil}; echo has(x, "key");') == "true"


def test_map_keys_after_clear(run_lox):
    assert (
        run_lox('var x = {"a": 1, "b": 2}; clear(x); var k = keys(x); echo len(k);')
        == "0"
    )


def test_map_values_after_clear(run_lox):
    assert (
        run_lox('var x = {"a": 1, "b": 2}; clear(x); var v = values(x); echo len(v);')
        == "0"
    )


def test_map_get_with_complex_default(run_lox):
    assert (
        run_lox(
            'var x = map(); var defaultMap = {"default": true}; echo get(x, "missing", defaultMap)["default"];'
        )
        == "true"
    )


def test_map_append_overwrite_behavior(run_lox):
    assert (
        run_lox(
            'var x = {"count": 1}; append(x, "count", 2); append(x, "count", 3); echo x["count"];'
        )
        == "3"
    )


def test_map_native_functions_with_numeric_keys(run_lox):
    assert (
        run_lox(
            """
            var x = map();
            append(x, 1, "one");
            append(x, 2, "two");
            echo to_string(has(x, 1)) + "," + get(x, 2, "missing") + "," + to_string(delete(x, 1));
        """
        )
        == "true,two,true"
    )


def test_map_native_functions_with_boolean_keys(run_lox):
    assert (
        run_lox(
            """
            var x = map();
            append(x, true, "yes");
            append(x, false, "no");
            echo get(x, true, "unknown") + "," + get(x, false, "unknown");
        """
        )
        == "yes,no"
    )


def test_map_complex_iteration_pattern(run_lox):
    assert (
        run_lox(
            """
            var scores = map();
            append(scores, "Alice", 95);
            append(scores, "Bob", 87);
            append(scores, "Charlie", 92);
            
            var names = keys(scores);
            var total = 0;
            var count = 0;
            
            for (var i = 0; i < len(names); i = i + 1) {
                var score = get(scores, names[i], 0);
                if (score > 90) {
                    total = total + score;
                    count = count + 1;
                }
            }
            
            var average = total / count;
            echo to_string(average);
        """
        )
        == "93.5"
    )


def test_map_len_function_invalid_argument(run_lox):
    try:
        run_lox("echo len(3);")
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_keys_function_invalid_argument(run_lox):
    try:
        run_lox("echo keys([1, 2, 3]);")
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_values_function_invalid_argument(run_lox):
    try:
        run_lox('echo values("not a map");')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_has_function_invalid_argument(run_lox):
    try:
        run_lox('echo has(42, "key");')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_delete_function_invalid_argument(run_lox):
    try:
        run_lox('echo delete([1, 2, 3], "key");')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_clear_function_invalid_argument(run_lox):
    try:
        run_lox('clear("not a map");')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_get_function_invalid_argument(run_lox):
    try:
        run_lox('echo get(true, "key", "default");')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_append_function_invalid_argument(run_lox):
    try:
        run_lox('append(nil, "key", "value");')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True
