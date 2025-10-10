def test_factorial_recursion(run_lox):
    assert (
        run_lox(
            """
    fun factorial(n) {
        if (n <= 1) return 1;
        return n * factorial(n - 1);
    }
    echo factorial(5);
    """
        )
        == "120"
    )


def test_fibonacci_recursion(run_lox):
    assert (
        run_lox(
            """
    fun fib(n) {
        if (n <= 1) return n;
        return fib(n - 1) + fib(n - 2);
    }
    echo fib(7);
    """
        )
        == "13"
    )


def test_countdown_recursion(run_lox):
    assert (
        run_lox(
            """
    fun countdown(n) {
        if (n <= 0) {
            echo "Done!";
            return;
        }
        echo n;
        countdown(n - 1);
    }
    countdown(3);
    """
        )
        == "3\n2\n1\nDone!"
    )


def test_sum_recursion(run_lox):
    assert (
        run_lox(
            """
    fun sum(n) {
        if (n <= 0) return 0;
        return n + sum(n - 1);
    }
    echo sum(4);
    """
        )
        == "10"
    )


def test_mutual_recursion(run_lox):
    assert (
        run_lox(
            """
    fun isEven(n) {
        if (n == 0) return true;
        return isOdd(n - 1);
    }
    
    fun isOdd(n) {
        if (n == 0) return false;
        return isEven(n - 1);
    }
    
    echo isEven(4);
    echo isOdd(3);
    """
        )
        == "true\ntrue"
    )
