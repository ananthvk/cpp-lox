def test_basic_while_loop(run_lox):
    assert (
        run_lox(
            """
    var i = 0;
    while (i < 3) {
        echo i;
        i = i + 1;
    }
    """
        )
        == "0\n1\n2"
    )


def test_while_loop_false_condition(run_lox):
    assert (
        run_lox(
            """
    while (false) {
        echo "never";
    }
    echo "done";
    """
        )
        == "done"
    )


def test_while_loop_with_local_variables(run_lox):
    assert (
        run_lox(
            """
    var x = 5;
    while (x > 0) {
        var y = x * 2;
        echo y;
        x = x - 1;
    }
    """
        )
        == "10\n8\n6\n4\n2"
    )


def test_nested_while_loops(run_lox):
    assert (
        run_lox(
            """
    var i = 1;
    while (i <= 2) {
        var j = 1;
        while (j <= 2) {
            echo i + j;
            j = j + 1;
        }
        i = i + 1;
    }
    """
        )
        == "2\n3\n3\n4"
    )


def test_while_loop_empty_body(run_lox):
    assert (
        run_lox(
            """
    var i = 0;
    while (i < 3) {
        i = i + 1;
    }
    echo i;
    """
        )
        == "3"
    )


def test_while_loop_variable_scope(run_lox):
    assert (
        run_lox(
            """
    var x = "outer";
    var i = 0;
    while (i < 2) {
        var x = "inner";
        echo x;
        i = i + 1;
    }
    echo x;
    """
        )
        == "inner\ninner\nouter"
    )


def test_basic_for_loop(run_lox):
    assert (
        run_lox(
            """
    for (var i = 0; i < 3; i = i + 1) {
        echo i;
    }
    """
        )
        == "0\n1\n2"
    )


def test_for_loop_no_initialization(run_lox):
    assert (
        run_lox(
            """
    var i = 0;
    for (; i < 3; i = i + 1) {
        echo i;
    }
    """
        )
        == "0\n1\n2"
    )



def test_for_loop_no_increment(run_lox):
    assert (
        run_lox(
            """
    for (var i = 0; i < 3;) {
        echo i;
        i = i + 1;
    }
    """
        )
        == "0\n1\n2"
    )


def test_nested_for_loops(run_lox):
    assert (
        run_lox(
            """
    for (var i = 1; i <= 2; i = i + 1) {
        for (var j = 1; j <= 2; j = j + 1) {
            echo i + j;
        }
    }
    """
        )
        == "2\n3\n3\n4"
    )
