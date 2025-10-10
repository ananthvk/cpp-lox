def test_basic_switch_case(run_lox):
    assert (
        run_lox(
            """
    var x = 2;
    switch (x) {
        case 1: echo "one";
        case 2: echo "two";
        case 3: echo "three";
        default: echo "other";
    }
    """
        )
        == "two"
    )


def test_switch_case_string(run_lox):
    assert (
        run_lox(
            """
    var fruit = "apple";
    switch (fruit) {
        case "banana": echo "yellow";
        case "apple": echo "red";
        case "orange": echo "orange";
        default: echo "unknown";
    }
    """
        )
        == "red"
    )


def test_switch_case_default(run_lox):
    assert (
        run_lox(
            """
    var x = 99;
    switch (x) {
        case 1: echo "one";
        case 2: echo "two";
        default: echo "default case";
    }
    """
        )
        == "default case"
    )


def test_switch_case_scoped_variables(run_lox):
    assert (
        run_lox(
            """
    var x = 1;
    switch (x) {
        case 1:
            var local = "case one";
            echo local;
        case 2:
            var local = "case two";
            echo local;
    }
    """
        )
        == "case one"
    )


def test_switch_case_no_match_no_default(run_lox):
    assert (
        run_lox(
            """
    var x = 5;
    switch (x) {
        case 1: echo "one";
        case 2: echo "two";
    }
    echo "after switch";
    """
        )
        == "after switch"
    )


def test_switch_case_expression_evaluation(run_lox):
    assert (
        run_lox(
            """
    var y = 2;
    switch (1 + 1) {
        case 1: echo "one";
        case 2: echo "match";
        case 3: echo "three";
    }
    """
        )
        == "match"
    )


def test_switch_case_scope_default_case(run_lox):
    assert (
        run_lox(
            """
    var x = 4;
    switch (x) {
        case 1:
            var local = "case one";
            echo local;
        case 2:
            var local = "case two";
            echo local;
        default:
            var local = "some other case";
            echo local;
    }
    """
        )
        == "some other case"
    )


def test_switch_case_scope_variable_isolation(run_lox):
    assert (
        run_lox(
            """
    var x = 1;
    var outside = "outside";
    switch (x) {
        case 1:
            var local = "case one";
            echo local;
        case 2:
            var local = "case two";
            echo local;
        default:
            var local = "some other case";
            echo local;
    }
    echo outside;
    """
        )
        == "case one\noutside"
    )


def test_switch_case_scope_same_variable_name(run_lox):
    assert (
        run_lox(
            """
    var local = "global local";
    var x = 2;
    switch (x) {
        case 1:
            var local = "case one";
            echo local;
        case 2:
            var local = "case two";
            echo local;
        default:
            var local = "some other case";
            echo local;
    }
    echo local;
    """
        )
        == "case two\nglobal local"
    )


def test_switch_case_scope_multiple_variables(run_lox):
    assert (
        run_lox(
            """
    var x = 1;
    switch (x) {
        case 1:
            var local = "case one";
            var another = "another var";
            echo local;
            echo another;
        case 2:
            var local = "case two";
            echo local;
        default:
            var local = "some other case";
            echo local;
    }
    """
        )
        == "case one\nanother var"
    )


def test_switch_case_scope_same_variable_name_const(run_lox):
    assert (
        run_lox(
            """
    const local = "global local";
    const x = 2;
    switch (x) {
        case 1:
            const local = "case one";
            echo local;
        case 2:
            const local = "case two";
            echo local;
        default:
            const local = "some other case";
            echo local;
    }
    echo local;
    """
        )
        == "case two\nglobal local"
    )
