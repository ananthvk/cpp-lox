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


def test_list_len_function(run_lox):
    assert run_lox("var x = [1, 2, 3, 4, 5]; echo len(x);") == "5"


def test_list_len_empty_list(run_lox):
    assert run_lox("var x = []; echo len(x);") == "0"


def test_list_cap_function(run_lox):
    assert run_lox("var x = [1, 2, 3]; echo cap(x);") == "3"


def test_list_constructor_with_length(run_lox):
    assert run_lox("var x = list(3); echo len(x);") == "3"


def test_list_constructor_with_length_and_default(run_lox):
    assert run_lox("var x = list(2, 42); echo x[1];") == "42"


def test_list_constructor_with_all_params(run_lox):
    assert run_lox("var x = list(2, 10, 5); echo len(x); echo cap(x);") == "2\n5"


def test_list_append_function(run_lox):
    assert run_lox("var x = [1, 2]; append(x, 3); echo x;") == "[1, 2, 3]"


def test_list_delete_function(run_lox):
    assert run_lox("var x = [1, 2, 3, 4]; delete(x, 1); echo x;") == "[1, 3, 4]"


def test_list_pop_function(run_lox):
    assert run_lox("var x = [1, 2, 3]; var y = pop(x); echo y; echo x;") == "3\n[1, 2]"


def test_list_operations_combined(run_lox):
    assert (
        run_lox(
            "var x = list(2, 5); append(x, 10); delete(x, 0); echo len(x); echo x[0];"
        )
        == "2\n5"
    )


def test_list_operations_in_loop(run_lox):
    code = """
    var x = [];
    var y = list(10, 0);
    
    // The first list will have all squares
    for(var i = 0; i < len(y); i = i + 1) {
        append(x, (i + 1) * (i + 1));
    }
    
    for(var i = len(x) - 1; i >= 0; i = i - 1) {
        y[len(y) - i - 1] = x[i];
    }
    
    assert(len(x) == 10, "x's length is not 10");
    assert(len(y) == 10, "y's length is not 10");
    
    echo x[9]; // Should be 100
    echo y[9]; // Should be 1

    const l2 = list(10, -1);

    for(var i = 0; i < 10; i = i + 1) {
        l2[i] = i;
    }

    for(var i = 3; i < 10; i = i + 1) {
        echo l2[i];
    }
    
    for(var i = 0; i < 10; i = i + 1) {
        echo pop(l2);
        assert(len(l2) == (10 - i - 1), "message");
    }
    
    var l3 = l2;
    l3 = list(10, 0);
    for(var i = 0; i < 10; i = i + 1) {
        l3[i] = i;
    }
    
    echo l3[5];
    delete(l3, 5);
    echo len(l3);
    echo l3[5];
    
    for(var i = 8; i >= 0; i = i - 1) {
        delete(l3, i);
    }
    echo len(l3);
    """
    assert (
        run_lox(code)
        == "100\n1\n3\n4\n5\n6\n7\n8\n9\n9\n8\n7\n6\n5\n4\n3\n2\n1\n0\n5\n9\n6\n0"
    )
