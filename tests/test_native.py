def test_sqrt_function(run_lox):
    assert run_lox("echo sqrt(16);") == "4"
    assert run_lox("echo sqrt(25);") == "5"
    assert run_lox("echo sqrt(0);") == "0"


def test_exit_function(run_lox):
    try:
        run_lox("exit(0);")
        assert False, "Should exit before reaching this"
    except Exception as e:
        pass


def test_print_function(run_lox):
    assert run_lox("print(42);") == "42"
    assert run_lox("print(42, 24);") == "42 24"
    assert run_lox('print("hello", "world");') == "hello world"


def test_println_function(run_lox):
    assert run_lox("println(42);print(3);") == "42\n3"
    assert run_lox("println(42, 24);print(3);") == "42 24\n3"
    assert run_lox('println("hello", "world");print(3);') == "hello world\n3"


def test_len_function(run_lox):
    assert run_lox('echo len("hello");') == "5"
    assert run_lox('echo len("");') == "0"
    assert run_lox('echo len("world");') == "5"


def test_to_string_function(run_lox):
    assert run_lox("echo to_string(42);") == "42"
    assert run_lox("echo to_string(3.14);") == "3.14"
    assert run_lox("echo to_string(true);") == "true"


def test_type_function(run_lox):
    assert run_lox("echo type(42);") == "int"
    assert run_lox("echo type(3.14);") == "double"
    assert run_lox('echo type("hello");') == "string"
    assert run_lox("echo type(true);") == "bool"


def test_rand_function(run_lox):
    result = float(run_lox("echo rand();"))
    assert 0 <= result < 1


def test_randint_function(run_lox):
    result = int(run_lox("echo randint(1, 10);"))
    assert 1 <= result <= 10
    result = int(run_lox("echo randint(5, 5);"))
    assert result == 5


def test_assert_function_true(run_lox):
    assert run_lox('assert(true, "should not fail");') == ""


def test_assert_function_false(run_lox):
    try:
        run_lox('assert(false, "test failure message");')
        assert False, "Should raise an error for false assertion"
    except Exception as e:
        assert "test failure message" in str(e)
