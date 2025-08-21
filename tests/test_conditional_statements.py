def test_basic_if_true(run_lox):
    assert (
        run_lox(
            """
    if (true) print "executed";
    """
        )
        == "executed"
    )


def test_basic_if_false(run_lox):
    assert (
        run_lox(
            """
    if (false) print "not executed";
    print "always executed";
    """
        )
        == "always executed"
    )


def test_if_else_true_condition(run_lox):
    assert (
        run_lox(
            """
    if (true) {
        print "if branch";
    } else {
        print "else branch";
    }
    """
        )
        == "if branch"
    )


def test_if_else_false_condition(run_lox):
    assert (
        run_lox(
            """
    if (false) {
        print "if branch";
    } else {
        print "else branch";
    }
    """
        )
        == "else branch"
    )


def test_if_with_expression_condition(run_lox):
    assert (
        run_lox(
            """
    var x = 5;
    if (x > 3) {
        print "x is greater than 3";
    }
    """
        )
        == "x is greater than 3"
    )


def test_nested_if_statements(run_lox):
    assert (
        run_lox(
            """
    var x = 10;
    if (x > 5) {
        if (x > 8) {
            print "x is greater than 8";
        } else {
            print "x is between 5 and 8";
        }
    }
    """
        )
        == "x is greater than 8"
    )


def test_if_else_chain(run_lox):
    assert (
        run_lox(
            """
    var score = 85;
    if (score >= 90) {
        print "A";
    } else if (score >= 80) {
        print "B";
    } else if (score >= 70) {
        print "C";
    } else {
        print "F";
    }
    """
        )
        == "B"
    )


def test_if_statement_with_block_scope(run_lox):
    assert (
        run_lox(
            """
    var x = "global";
    if (true) {
        var x = "local";
        print x;
    }
    print x;
    """
        )
        == """local
global"""
    )
