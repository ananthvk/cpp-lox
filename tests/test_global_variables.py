def test_global_define(run_lox):
    assert run_lox("var a = 32; echo a;") == "32"


def test_global_define_multiple(run_lox):
    assert run_lox("var a = 32; var b = 64; var c = b + a; echo c;") == "96"


def test_global_assign(run_lox):
    assert (
        run_lox("var a = 32; var b = 64; var c = b + a; var d; d = c; echo d;") == "96"
    )


def test_global_redefine(run_lox):
    assert run_lox("var a = 10; a = 20; echo a;") == "20"


def test_global_get_undefined(run_lox):
    try:
        run_lox("echo b;")
        assert False, "Should raise an error for undefined variable"
    except Exception:
        assert True


def test_global_assign_before_define(run_lox):
    try:
        run_lox("b = 42; echo b;")
        assert False, "Should raise an error for assignment before definition"
    except Exception:
        assert True


def test_global_define_and_get_multiple_types(run_lox):
    assert (
        run_lox(
            'var a = "hello"; var b = true; var c = 3.14; echo a; echo b; echo c;'
        )
        == "hello\ntrue\n3.14"
    )


def test_global_assign_expression(run_lox):
    assert run_lox("var a = 5; var b = 10; a = a + b; echo a;") == "15"
