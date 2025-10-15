def test_closure_with_parameter(run_lox):
    assert (
        run_lox(
            """
    fun outer(msg) {
        fun inner() {
            echo msg;
        }
        inner();
    }
    outer("world");
    """
        )
        == "world"
    )

def test_nested_closure_access(run_lox):
    assert (
        run_lox(
            """
    fun level1() {
        var x = 42;
        fun level2() {
            var y = 10;
            fun level3() {
                echo x + y;
            }
            level3();
        }
        level2();
    }
    level1();
    """
        )
        == "52"
    )

def test_closure_with_local_modification(run_lox):
    assert (
        run_lox(
            """
    fun outer() {
        var count = 0;
        fun increment() {
            count = count + 1;
            echo count;
        }
        increment();
        increment();
    }
    outer();
    """
        )
        == "1\n2"
    )

def test_multiple_closures_same_scope(run_lox):
    assert (
        run_lox(
            """
    fun outer() {
        var value = "shared";
        fun first() {
            echo value + "1";
        }
        fun second() {
            echo value + "2";
        }
        first();
        second();
    }
    outer();
    """
        )
        == "shared1\nshared2"
    )

def test_closure_with_conditional(run_lox):
    assert (
        run_lox(
            """
    fun test(flag) {
        var secret = "hidden";
        if (flag) {
            fun reveal() {
                echo secret;
            }
            reveal();
        }
    }
    test(true);
    """
        )
        == "hidden"
    )
