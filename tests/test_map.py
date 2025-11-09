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


def test_map_list_as_key_error(run_lox):
    try:
        run_lox('var x = {}; var key = [1, 2, 3]; x[key] = "value";')
        assert False, "Expected CalledProcessError"
    except subprocess.CalledProcessError:
        assert True


def test_map_map_as_key_error(run_lox):
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
