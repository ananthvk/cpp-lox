import math


def test_addition(run_lox):
    assert run_lox("1 + 2") == "3"


def test_subtraction(run_lox):
    assert run_lox("5 - 3") == "2"


def test_multiplication(run_lox):
    assert run_lox("4 * 2") == "8"


def test_division(run_lox):
    assert run_lox("8 / 2") == "4"


def test_grouping(run_lox):
    assert run_lox("(1 + 2) * 3") == "9"


def test_negation(run_lox):
    assert run_lox("-5") == "-5"


def test_comparison_greater(run_lox):
    assert run_lox("5 > 3") == "true"


def test_comparison_less(run_lox):
    assert run_lox("3 < 5") == "true"


def test_comparison_equal(run_lox):
    assert run_lox("4 == 4") == "true"


def test_comparison_not_equal(run_lox):
    assert run_lox("4 != 5") == "true"


def test_string_concatenation(run_lox):
    assert run_lox('"Hello, " + "world!"') == "Hello, world!"


def test_expression(run_lox):
    assert math.floor(
        float(
            run_lox(
                "61*(-69/(-19*83-(76/(((-12/-10-(-80/-63-((((((((-14*(9*-42-(-8/-11-91/(-82*(-30*-39-4/16-61/-71--19/64)--65*62+-99*-89-54/98)+-24*-67-27/92)/34--77*41-43*-100)--35*75+20*75--76/58)*-36--23/-12+64*70-74*-93)*-56--84/-92+-63/-8-83*-26)/-1-62*-55+-26*-71--21*44)/88--87*16+76/-18--65/83)*96--19*-9+24/-53--2/49)/64-40*62--16/-77--82*-15)*-49--35/20+-2*89--33/28)/-100-97/14-56/45)/99+-43*68-41/47)*32-53*-12+-15/-39--33*-14)*-82-44/-22-62/23-53*79)--3/33+72*-11-98*-51)/60--37/31--91*30)-87/-65--97*-26-26/-40)-86/-58+-16*-68-60*-80"
            )
        )
    ) == math.floor(float("-147835.10359923032"))
