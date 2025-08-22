def test_basic_switch_case(run_lox):
    assert (
        run_lox(
            """
    var x = 2;
    switch (x) {
        case 1: print "one";
        case 2: print "two";
        case 3: print "three";
        default: print "other";
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
        case "banana": print "yellow";
        case "apple": print "red";
        case "orange": print "orange";
        default: print "unknown";
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
        case 1: print "one";
        case 2: print "two";
        default: print "default case";
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
            print local;
        case 2:
            var local = "case two";
            print local;
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
        case 1: print "one";
        case 2: print "two";
    }
    print "after switch";
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
        case 1: print "one";
        case 2: print "match";
        case 3: print "three";
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
            print local;
        case 2:
            var local = "case two";
            print local;
        default:
            var local = "some other case";
            print local;
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
            print local;
        case 2:
            var local = "case two";
            print local;
        default:
            var local = "some other case";
            print local;
    }
    print outside;
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
            print local;
        case 2:
            var local = "case two";
            print local;
        default:
            var local = "some other case";
            print local;
    }
    print local;
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
            print local;
            print another;
        case 2:
            var local = "case two";
            print local;
        default:
            var local = "some other case";
            print local;
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
            print local;
        case 2:
            const local = "case two";
            print local;
        default:
            const local = "some other case";
            print local;
    }
    print local;
    """
        )
        == "case two\nglobal local"
    )
