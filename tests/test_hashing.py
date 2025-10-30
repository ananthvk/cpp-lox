from subprocess import CalledProcessError


def test_hash_same_integers(run_lox):
    """Test that same integers produce same hash"""
    assert run_lox("echo hash(42) == hash(42);") == "true"


def test_hash_same_strings(run_lox):
    """Test that same strings produce same hash"""
    assert run_lox('echo hash("hello") == hash("hello");') == "true"


def test_hash_same_booleans_true(run_lox):
    """Test that same boolean true values produce same hash"""
    assert run_lox("echo hash(true) == hash(true);") == "true"


def test_hash_same_booleans_false(run_lox):
    """Test that same boolean false values produce same hash"""
    assert run_lox("echo hash(false) == hash(false);") == "true"


def test_hash_same_nil(run_lox):
    """Test that nil produces same hash"""
    assert run_lox("echo hash(nil) == hash(nil);") == "true"


def test_hash_different_integers(run_lox):
    """Test that different integers produce different hashes"""
    assert run_lox("echo hash(42) == hash(43);") == "false"


def test_hash_different_strings(run_lox):
    """Test that different strings produce different hashes"""
    assert run_lox('echo hash("hello") == hash("world");') == "false"


def test_hash_consistent_across_variables(run_lox):
    """Test that hash is consistent when stored in variables"""
    code = """
    var x = 123;
    var y = 123;
    echo hash(x) == hash(y);
    """
    assert run_lox(code) == "true"


def test_hash_with_expressions(run_lox):
    """Test that hash works with expression results"""
    assert run_lox("echo hash(10 + 5) == hash(15);") == "true"


def test_hash_zero_integer(run_lox):
    """Test that zero hashes consistently"""
    assert run_lox("echo hash(0) == hash(0);") == "true"


def test_hash_negative_integers(run_lox):
    """Test that negative integers hash consistently"""
    assert run_lox("echo hash(-42) == hash(-42);") == "true"


def test_hash_empty_string(run_lox):
    """Test that empty strings hash consistently"""
    assert run_lox('echo hash("") == hash("");') == "true"


def test_hash_string_with_spaces(run_lox):
    """Test that strings with spaces hash consistently"""
    assert run_lox('echo hash("hello world") == hash("hello world");') == "true"


def test_hash_string_with_numbers(run_lox):
    """Test that strings containing numbers hash consistently"""
    assert run_lox('echo hash("test123") == hash("test123");') == "true"


def test_hash_large_integer(run_lox):
    """Test that large integers hash consistently"""
    assert run_lox("echo hash(999999) == hash(999999);") == "true"


def test_hash_decimal_numbers(run_lox):
    """Test that decimal numbers hash consistently"""
    assert run_lox("echo hash(3.14) == hash(3.14);") == "true"


def test_hash_variable_reassignment(run_lox):
    """Test that hash remains consistent after variable reassignment"""
    code = """
    var x = 100;
    var h1 = hash(x);
    x = 200;
    x = 100;
    var h2 = hash(x);
    echo h1 == h2;
    """
    assert run_lox(code) == "true"


def test_hash_boolean_comparison(run_lox):
    """Test that true and false have different hashes"""
    assert run_lox("echo hash(true) == hash(false);") == "false"


def test_hash_string_case_sensitivity(run_lox):
    """Test that string hashing is case sensitive"""
    assert run_lox('echo hash("Hello") == hash("hello");') == "false"


def test_hash_concatenated_strings(run_lox):
    """Test that string concatenation produces consistent hash"""
    code = """
    var str1 = "hello";
    var str2 = "world";
    var combined = str1 + str2;
    echo hash(combined) == hash("helloworld");
    """
    assert run_lox(code) == "true"


def test_hash_computed_values(run_lox):
    """Test that computed values hash consistently"""
    assert run_lox("echo hash(2 * 3 * 7) == hash(42);") == "true"


def test_hash_multiple_comparisons(run_lox):
    """Test multiple hash comparisons in sequence"""
    code = """
    var a = hash(1);
    var b = hash(1);
    var c = hash(2);
    echo a == b;
    echo a == c;
    """
    assert run_lox(code) == "true\nfalse"


def test_hash_nil_vs_zero(run_lox):
    """Test that nil and zero have different hashes"""
    assert run_lox("echo hash(nil) == hash(0);") == "false"


def test_hash_stored_and_immediate(run_lox):
    """Test that stored hash equals immediate hash calculation"""
    code = """
    var value = 42;
    var stored_hash = hash(value);
    echo stored_hash == hash(42);
    """


def test_hash_nested_expressions(run_lox):
    """Test that deeply nested expressions hash consistently"""
    code = """
    var result1 = hash((2 + 3) * (4 - 1));
    var result2 = hash(5 * 3);
    echo result1 == result2;
    """
    assert run_lox(code) == "true"


def test_hash_string_escape_sequences(run_lox):
    """Test that strings with escape sequences hash consistently"""
    assert run_lox(r'echo hash("hello\nworld") == hash("hello\nworld");') == "true"


def test_hash_floating_point_precision(run_lox):
    """Test that floating point numbers with different representations hash differently"""
    assert run_lox("echo hash(1.0) == hash(1.00000001);") == "false"


def test_hash_arithmetic_operations_order(run_lox):
    """Test that mathematically equivalent expressions hash the same"""
    code = """
    var hash1 = hash(2 * 3 + 4);
    var hash2 = hash(4 + 2 * 3);
    var hash3 = hash(10);
    echo hash1 == hash2;
    echo hash1 == hash3;
    """
    assert run_lox(code) == "true\ntrue"


def test_hash_string_interpolation_vs_literal(run_lox):
    """Test that string concatenation vs literal produces same hash"""
    code = """
    var num = 42;
    var str1 = "value" + "42";
    var str2 = "value42";
    echo hash(str1) == hash(str2);
    """
    assert run_lox(code) == "true"


def test_hash_boolean_expressions(run_lox):
    """Test that boolean expressions hash consistently"""
    code = """
    var result1 = hash(true and true);
    var result2 = hash(true);
    var result3 = hash(false or true);
    echo result1 == result2;
    echo result1 == result3;
    """
    assert run_lox(code) == "true\ntrue"


def test_hash_comparison_results(run_lox):
    """Test that comparison results hash consistently"""
    code = """
    var comp1 = hash(5 > 3);
    var comp2 = hash(10 < 20);
    var comp3 = hash(true);
    echo comp1 == comp2;
    echo comp1 == comp3;
    """
    assert run_lox(code) == "true\ntrue"


def test_hash_variable_chains(run_lox):
    """Test that variable assignment chains preserve hash consistency"""
    code = """
    var a = 100;
    var b = a;
    var c = b;
    echo hash(a) == hash(b);
    echo hash(b) == hash(c);
    echo hash(a) == hash(100);
    """
    assert run_lox(code) == "true\ntrue\ntrue"


def test_hash_mixed_type_inequality(run_lox):
    """Test that different types with same string representation have different hashes"""
    code = """
    echo hash(42) == hash("42");
    echo hash(true) == hash("true");
    echo hash(false) == hash("false");
    """
    assert run_lox(code) == "false\nfalse\nfalse"


def test_hash_recursive_calculations(run_lox):
    """Test that recursive mathematical calculations hash consistently"""
    code = """
    var factorial5_1 = hash(5 * 4 * 3 * 2 * 1);
    var factorial5_2 = hash(120);
    var fibonacci7_1 = hash(1 + 1 + 2 + 3 + 5 + 8);
    var fibonacci7_2 = hash(20);
    echo factorial5_1 == factorial5_2;
    echo fibonacci7_1 == fibonacci7_2;
    """
    assert run_lox(code) == "true\ntrue"


def test_hash_string_length_variations(run_lox):
    """Test that strings of different lengths hash differently"""
    code = """
    echo hash("a") == hash("aa");
    echo hash("test") == hash("testing");
    echo hash("x") == hash("");
    """
    assert run_lox(code) == "false\nfalse\nfalse"


def test_hash_numeric_boundaries(run_lox):
    """Test hash behavior at numeric boundaries"""
    code = """
    echo hash(-1) == hash(1);
    echo hash(0) == hash(-0);
    """
    assert run_lox(code) == "false\ntrue"


def test_hash_complex_boolean_logic(run_lox):
    """Test that complex boolean expressions hash consistently"""
    code = """
    var expr1 = hash((true and false) or (false or true));
    var expr2 = hash(true);
    var expr3 = hash((5 > 3) and (2 < 4));
    echo expr1 == expr2;
    echo expr2 == expr3;
    """
    assert run_lox(code) == "true\ntrue"


def test_hash_string_with_special_characters(run_lox):
    """Test that strings with special characters hash consistently"""
    code = """
    var special1 = hash("!@#$%^&*()");
    var special2 = hash("!@#$%^&*()");
    var special3 = hash("hello, world!");
    var special4 = hash("hello, world!");
    echo special1 == special2;
    echo special3 == special4;
    echo special1 == special3;
    """
    assert run_lox(code) == "true\ntrue\nfalse"


def test_hash_temporal_consistency(run_lox):
    """Test that hash values remain consistent across multiple calculations"""
    code = """
    var value = 42;
    var hash1 = hash(value);
    var hash2 = hash(value);
    var hash3 = hash(42);
    var temp = value + 1 - 1;
    var hash4 = hash(temp);
    echo hash1 == hash2;
    echo hash1 == hash3;
    echo hash1 == hash4;
    """
    assert run_lox(code) == "true\ntrue\ntrue"


def test_hash_list_should_fail(run_lox):
    """Test that hashing a list should fail"""
    try:
        run_lox("var x = [1, 2, 3]; echo hash(x);")
        assert False
    except CalledProcessError:
        assert True
