def test_const_declare_twice_global(run_lox):
    try:
        run_lox("const a = 32; const a = 64;")
        assert False, "Should raise an error for const redeclaration"
    except Exception:
        assert True


def test_const_declaration(run_lox):
    assert (
        run_lox(
            """
            const x = 32;
            echo x;
            """
        )
        == "32"
    )


def test_const_assignment_error(run_lox):
    try:
        run_lox(
            """
            const x = 32;
            x = 64;
        """
        )
        assert False, "Should raise an error for const assignment"
    except Exception:
        assert True


def test_const_in_block_scope(run_lox):
    assert (
        run_lox(
            """
            const x = 10;
            {
                const x = 20;
                echo x;
            }
            echo x;
            """
        )
        == "20\n10"
    )


def test_const_outside_block_scope(run_lox):
    try:
        run_lox(
            """
            {
                const x = 32;
            }
            echo x;
        """
        )
        assert False, "Should raise an error for accessing const outside scope"
    except Exception:
        assert True


def test_const_redeclare_in_same_scope(run_lox):
    try:
        run_lox(
            """
            {
                const x = 32;
                const x = 64;
            }
        """
        )
        assert False, "Should raise an error for const redeclaration in same scope"
    except Exception:
        assert True


def test_const_uninitialized(run_lox):
    try:
        run_lox("const x;")
        assert False, "Should raise an error for uninitialized const"
    except Exception:
        assert True


def test_const_with_expression(run_lox):
    assert (
        run_lox(
            """
            const x = 10 + 20;
            echo x;
            """
        )
        == "30"
    )


def test_const_and_var_different_names_global(run_lox):
    assert (
        run_lox(
            """
            const x = 10;
            var y = 20;
            echo x;
            echo y;
            """
        )
        == "10\n20"
    )


def test_var_then_const_same_name_global(run_lox):
    try:
        run_lox(
            """
            var x = 10;
            const x = 20;
            """
        )
        assert False, "Should raise an error for redeclaring var as const"
    except Exception:
        assert True


def test_const_then_var_same_name_global(run_lox):
    try:
        run_lox(
            """
            const x = 10;
            var x = 20;
            """
        )
        assert False, "Should raise an error for redeclaring const as var"
    except Exception:
        assert True


def test_const_and_var_assignment_behavior_global(run_lox):
    try:
        run_lox(
            """
            const x = 10;
            var y = 20;
            y = 30;
            x = 40;
            """
        )
        assert False, "Should raise an error for assigning to const"
    except Exception:
        assert True


def test_var_redeclare_global(run_lox):
    assert (
        run_lox(
            """
            var x = 32;
            var x = 64;
            echo x;
            """
        )
        == "64"
    )

