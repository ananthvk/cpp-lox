def test_logical_operators_basic(run_lox):
    expressions = [
        "False and False",
        "False and True",
        "True and False",
        "True and True",
        "False or False",
        "False or True",
        "True or False",
        "True or True",
    ]
    for expression in expressions:
        assert str(eval(expression)).lower() == run_lox(f"echo {expression.lower()};")


def test_logical_operators_short_circuit_and(run_lox):
    # Test short-circuiting with 'and' - second operand shouldn't be evaluated if first is false
    expressions = [
        "false and false",
        "false and true",
        "nil and true",
        "0 and true",  # Note: 0 is truthy in Lox so the expression evaluates to RHS (true)
        '"" and true',
    ]
    for expression in expressions:
        result = run_lox(f"echo {expression};")
        assert result in ["false", "nil", "true", ""]


def test_logical_operators_short_circuit_or(run_lox):
    # Test short-circuiting with 'or' - second operand shouldn't be evaluated if first is true
    expressions = [
        "true or false",
        "true or true",
        '"hello" or false',
        "1 or false",
        "42 or nil",
    ]
    for expression in expressions:
        result = run_lox(f"echo {expression};")
        assert result in ["true", "hello", "1", "42"]


def test_logical_operators_falsy_values(run_lox):
    # Test with falsy values (false and nil in Lox)
    expressions = [
        "nil or false",
        "false or nil",
        "nil and true",
        "false and 42",
        "nil or nil",
        "false or false",
    ]
    expected = ["false", "nil", "nil", "false", "nil", "false"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result


def test_logical_operators_truthy_values(run_lox):
    # Test with truthy values (everything except false and nil)
    expressions = [
        '0 or "default"',
        '"" or "default"',
        "0 and 42",
        '"hello" and "world"',
        "123 or false",
        '-1 and "negative"',
    ]
    expected = ["0", "", "42", "world", "123", "negative"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result


def test_logical_operators_chaining(run_lox):
    # Test chaining multiple logical operators
    expressions = [
        "true and true and true",
        "true and true and false",
        "false or false or true",
        "false or false or false",
        "true and false or true",
        "false or true and false",
    ]
    expected = ["true", "false", "true", "false", "true", "false"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result


def test_logical_operators_mixed_types(run_lox):
    # Test logical operators with mixed data types
    expressions = [
        "42 and true",
        '"string" or false',
        "nil and 123",
        "0 or nil",
        "true and 3.14",
        'false or "fallback"',
    ]
    expected = ["true", "string", "nil", "0", "3.14", "fallback"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result


def test_logical_operators_precedence(run_lox):
    # Test operator precedence (and has higher precedence than or)
    expressions = [
        "true or false and false",  # Should be: true or (false and false) = true
        "false and true or true",  # Should be: (false and true) or true = true
        "false or true and true",  # Should be: false or (true and true) = true
        "true and false or false",  # Should be: (true and false) or false = false
    ]
    expected = ["true", "true", "true", "false"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result


def test_not_operator_basic(run_lox):
    # Test basic not operator functionality
    expressions = [
        "not true",
        "not false",
        "not nil",
        "not 0",
        "not 42",
        'not ""',
        'not "hello"',
    ]
    expected = ["false", "true", "true", "false", "false", "false", "false"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result


def test_not_operator_double_negation(run_lox):
    # Test double negation
    expressions = [
        "not not true",
        "not not false",
        "not not nil",
        "not not 42",
        'not not "test"',
    ]
    expected = ["true", "false", "false", "true", "true"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result


def test_not_operator_with_logical_combinations(run_lox):
    # Test not operator combined with and/or
    expressions = [
        "not true and false",
        "not false or true",
        "not (true and false)",
        "not (false or true)",
        "true and not false",
        "false or not nil",
    ]
    expected = ["false", "true", "true", "false", "true", "true"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result


def test_not_operator_precedence(run_lox):
    # Test not operator precedence (higher than and/or)
    expressions = [
        "not true and true",  # Should be: (not true) and true = false
        "not false or false",  # Should be: (not false) or false = true
        "true and not false",  # Should be: true and (not false) = true
        "false or not nil",  # Should be: false or (not nil) = true
    ]
    expected = ["false", "true", "true", "true"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result


def test_not_operator_complex_expressions(run_lox):
    # Test not operator in complex expressions
    expressions = [
        "not (true and false) or not (false or true)",
        "not true and not false",
        "not (42 and nil)",
        'not ("" or 0)',
        "not not not true",
    ]
    expected = ["true", "false", "true", "false", "false"]
    for expression, expected_result in zip(expressions, expected):
        assert run_lox(f"echo {expression};") == expected_result
