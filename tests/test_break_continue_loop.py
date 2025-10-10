
def test_continue_in_for_loop_basic(run_lox):
    assert (
        run_lox(
            """
    for (var i = 1; i <= 5; i = i + 1) {
        if (i == 3) continue;
        echo i;
    }
    """
        )
        == "1\n2\n4\n5"
    )


def test_continue_multiple_conditions_while(run_lox):
    assert (
        run_lox(
            """
    var i = 0;
    while (i < 10) {
        i = i + 1;
        if (i == 2) continue;
        if (i == 5) continue;
        if (i == 8) continue;
        echo i;
    }
    """
        )
        == "1\n3\n4\n6\n7\n9\n10"
    )


def test_continue_in_for_loop_skip_even(run_lox):
    assert (
        run_lox(
            """
    for (var i = 1; i <= 6; i = i + 1) {
        if (i == 2 or i == 4 or i == 6) continue;
        echo i;
    }
    """
        )
        == "1\n3\n5"
    )


def test_continue_nested_while_outer(run_lox):
    assert (
        run_lox(
            """
    var i = 0;
    while (i < 3) {
        i = i + 1;
        if (i == 2) continue;
        var j = 1;
        while (j <= 2) {
            echo i;
            echo j;
            j = j + 1;
        }
    }
    """
        )
        == "1\n1\n1\n2\n3\n1\n3\n2"
    )


def test_continue_nested_while_inner(run_lox):
    assert (
        run_lox(
            """
    var i = 1;
    while (i <= 2) {
        var j = 1;
        while (j <= 3) {
            if (j == 2) {
                j = j + 1;
                continue;
            }
            echo i;
            echo j;
            j = j + 1;
        }
        i = i + 1;
    }
    """
        )
        == "1\n1\n1\n3\n2\n1\n2\n3"
    )


def test_continue_with_complex_condition_for(run_lox):
    assert (
        run_lox(
            """
    for (var i = 1; i <= 8; i = i + 1) {
        if (i > 2 and i < 6) continue;
        echo i;
    }
    """
        )
        == "1\n2\n6\n7\n8"
    )


def test_continue_at_loop_end_while(run_lox):
    assert (
        run_lox(
            """
    var i = 0;
    while (i < 4) {
        i = i + 1;
        echo i;
        if (i == 4) continue;
        echo "after";
    }
    """
        )
        == "1\nafter\n2\nafter\n3\nafter\n4"
    )


def test_continue_with_variable_assignment_for(run_lox):
    assert (
        run_lox(
            """
    var sum = 0;
    for (var i = 1; i <= 5; i = i + 1) {
        if (i == 3) {
            sum = sum + 100;
            continue;
        }
        sum = sum + i;
        echo sum;
    }
    """
        )
        == "1\n3\n107\n112"
    )


def test_continue_all_iterations_while(run_lox):
    assert (
        run_lox(
            """
    var i = 0;
    while (i < 3) {
        i = i + 1;
        continue;
        echo "never printed";
    }
    echo "done";
    """
        )
        == "done"
    )
