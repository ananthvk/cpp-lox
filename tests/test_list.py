def test_list_creation(run_lox):
    assert run_lox("var x = [1, 2, 3]; echo x;") == "[1, 2, 3]"


def test_list_get_element(run_lox):
    assert run_lox("var x = [10, 20, 30]; echo x[1];") == "20"


def test_list_set_element(run_lox):
    assert run_lox("var x = [1, 2, 3]; x[1] = 42; echo x[1];") == "42"


def test_list_mixed_types(run_lox):
    assert run_lox('var x = [1, "hello", true]; echo x[1];') == "hello"


def test_list_zero_index(run_lox):
    assert run_lox("var x = [100, 200, 300]; echo x[0];") == "100"


def test_nested_lists(run_lox):
    assert run_lox("var x = [[1, 2], [3, 4]]; echo x[0][1];") == "2"


def test_list_assignment_chain(run_lox):
    assert (
        run_lox("var x = [1, 2, 3]; x[0] = x[1] = 5; echo x[0]; echo x[1];") == "5\n5"
    )


def test_list_multiple_assignments(run_lox):
    assert run_lox("var x = [1, 2, 3]; x[0] = 10; x[2] = 30; echo x;") == "[10, 2, 30]"


def test_list_with_variables(run_lox):
    assert run_lox("var a = 5; var b = 10; var x = [a, b, a + b]; echo x[2];") == "15"


def test_list_reassign_entire_list(run_lox):
    assert run_lox("var x = [1, 2, 3]; x = [4, 5, 6]; echo x[1];") == "5"


def test_empty_list(run_lox):
    assert run_lox("var x = []; echo x;") == "[]"


def test_single_element_list(run_lox):
    assert run_lox("var x = [42]; echo x[0];") == "42"


def test_list_with_expressions(run_lox):
    assert run_lox("var x = [1 + 2, 3 * 4, 5 - 1]; echo x[1];") == "12"


def test_list_modify_then_access(run_lox):
    assert run_lox("var x = [1, 2, 3]; x[1] = x[0] + x[2]; echo x[1];") == "4"


def test_list_boolean_and_nil_values(run_lox):
    assert run_lox("var x = [true, false, nil]; echo x[2];") == "nil"
