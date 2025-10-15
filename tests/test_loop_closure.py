def test_for_loop_does_not_create_new_variable_every_iteration(run_lox):
    # TODO: Later change the loop to create a new variable every iteration
    assert (
        run_lox(
            """
    var inner0;
    var inner1;
    var inner2;

    for (var i = 0; i < 3; i = i + 1) {
        fun inner() {
            println(i);
        }
        if(i == 0)
            inner0 = inner;
        else if(i == 1)
            inner1 = inner;
        else
            inner2 = inner;
    }

    inner0();
    inner1();
    inner2();
    """
        )
        == "3\n3\n3"
    )
