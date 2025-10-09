def test_basic_function_declaration(run_lox):
    assert (
        run_lox(
            """
    fun greet() {
        print "Hello!";
    }
    greet();
    """
        )
        == "Hello!"
    )


def test_function_with_parameters(run_lox):
    assert (
        run_lox(
            """
    fun add(a, b) {
        print a + b;
    }
    add(3, 5);
    """
        )
        == "8"
    )


def test_function_with_return_value(run_lox):
    assert (
        run_lox(
            """
    fun multiply(a, b) {
        return a * b;
    }
    print multiply(4, 6);
    """
        )
        == "24"
    )


def test_function_with_multiple_parameters(run_lox):
    assert (
        run_lox(
            """
    fun concat(a, b, c) {
        return a + b + c;
    }
    print concat("Hello", " ", "World");
    """
        )
        == "Hello World"
    )


def test_function_with_local_variables(run_lox):
    assert (
        run_lox(
            """
    fun calculate() {
        var x = 10;
        var y = 20;
        return x + y;
    }
    print calculate();
    """
        )
        == "30"
    )


def test_function_calling_another_function(run_lox):
    assert (
        run_lox(
            """
    fun double(n) {
        return n * 2;
    }
    fun quadruple(n) {
        return double(double(n));
    }
    print quadruple(3);
    """
        )
        == "12"
    )


def test_recursive_function_factorial(run_lox):
    assert (
        run_lox(
            """
    fun factorial(n) {
        if (n <= 1) {
            return 1;
        }
        return n * factorial(n - 1);
    }
    print factorial(5);
    """
        )
        == "120"
    )


def test_function_with_no_return_statement(run_lox):
    assert (
        run_lox(
            """
    fun printNumber(n) {
        print n;
    }
    printNumber(42);
    """
        )
        == "42"
    )


def test_function_with_early_return(run_lox):
    assert (
        run_lox(
            """
    fun checkSign(n) {
        if (n > 0) {
            return "positive";
        }
        return "not positive";
    }
    print checkSign(5);
    print checkSign(-3);
    """
        )
        == "positive\nnot positive"
    )


def test_function_with_conditional_logic(run_lox):
    assert (
        run_lox(
            """
    fun max(a, b) {
        if (a > b) {
            return a;
        } else {
            return b;
        }
    }
    print max(10, 15);
    """
        )
        == "15"
    )


def test_function_with_loop(run_lox):
    assert (
        run_lox(
            """
    fun countdown(n) {
        while (n > 0) {
            print n;
            n = n - 1;
        }
    }
    countdown(3);
    """
        )
        == "3\n2\n1"
    )


def test_function_with_string_operations(run_lox):
    assert (
        run_lox(
            """
    fun greetUser(name) {
        return "Hello, " + name + "!";
    }
    print greetUser("Alice");
    """
        )
        == "Hello, Alice!"
    )


def test_function_modifying_global_variable(run_lox):
    assert (
        run_lox(
            """
    var counter = 0;
    fun increment() {
        counter = counter + 1;
        print counter;
    }
    increment();
    increment();
    """
        )
        == "1\n2"
    )


def test_function_with_boolean_logic(run_lox):
    assert (
        run_lox(
            """
    fun isPositive(n) {
        return n >= 0;
    }
    print isPositive(1);
    print isPositive(-5);
    """
        )
        == "true\nfalse"
    )


def test_nested_function_calls(run_lox):
    assert (
        run_lox(
            """
    fun add(a, b) {
        return a + b;
    }
    fun multiply(a, b) {
        return a * b;
    }
    print add(multiply(2, 3), multiply(4, 5));
    """
        )
        == "26"
    )


def test_scope(run_lox):
    assert (
        run_lox(
            """
    var global = 3;
    fun dosomething(y) {
        global = global + y;
    }
    fun another(x) {
        dosomething(x * 3);
    }
    another(5);
    print global;
            """
        )
        == "18"
    )
