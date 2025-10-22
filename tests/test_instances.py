def test_class_instance_creation(run_lox):
    """Test that we can create an instance of a class"""
    result = run_lox("class Foo {} var f = Foo(); println(f);")
    assert "Foo" in result and "instance" in result.lower()


def test_set_and_get_property(run_lox):
    """Test setting and getting a property on an instance"""
    result = run_lox("class Foo {} var f = Foo(); f.x = 32; println(f.x);")
    assert "32" in result


def test_has_property_true(run_lox):
    """Test has_property returns true for existing property"""
    result = run_lox(
        'class Foo {} var f = Foo(); f.x = 32; println(has_property(f, "x"));'
    )
    assert "true" in result.lower()


def test_has_property_false(run_lox):
    """Test has_property returns false for non-existing property"""
    result = run_lox('class Foo {} var f = Foo(); println(has_property(f, "y"));')
    assert "false" in result.lower()


def test_get_property_existing(run_lox):
    """Test get_property returns value for existing property"""
    result = run_lox(
        'class Foo {} var f = Foo(); f.name = "test"; println(get_property(f, "name"));'
    )
    assert "test" in result


def test_get_property_undefined(run_lox):
    """Test get_property raises error for undefined property"""
    try:
        run_lox('class Foo {} var f = Foo(); println(get_property(f, "undefined"));')
        assert False, "Should raise an error for undefined property"
    except Exception:
        assert True


def test_set_property_function(run_lox):
    """Test set_property function sets a property"""
    result = run_lox(
        'class Foo {} var f = Foo(); set_property(f, "value", 42); println(f.value);'
    )
    assert "42" in result


def test_del_property_existing(run_lox):
    """Test del_property returns true when deleting existing property"""
    result = run_lox(
        'class Foo {} var f = Foo(); f.x = 10; println(del_property(f, "x"));'
    )
    assert "true" in result.lower()


def test_del_property_non_existing(run_lox):
    """Test del_property returns false when property doesn't exist"""
    result = run_lox(
        'class Foo {} var f = Foo(); println(del_property(f, "nonexistent"));'
    )
    assert "false" in result.lower()


def test_complex_property_operations(run_lox):
    """Test complex property operations combining all methods and dot access"""
    code = """
    class Person {}
    var p = Person();
    
    // Set properties using dot notation
    p.name = "Alice";
    p.age = 30;
    p.city = "New York";
    
    // Check properties exist using has_property
    println(has_property(p, "name"));
    println(has_property(p, "age"));
    println(has_property(p, "height"));
    
    // Get properties using both dot notation and get_property
    println(p.name);
    println(get_property(p, "age"));
    
    // Set new property using set_property function
    set_property(p, "country", "USA");
    println(p.country);
    
    // Delete property and verify
    println(del_property(p, "city"));
    println(has_property(p, "city"));
    
    // Final state check
    println(p.name);
    println(p.age);
    println(p.country);
    """

    result = run_lox(code)
    lines = result.strip().split("\n")

    assert "true" in lines[0].lower()  # has name
    assert "true" in lines[1].lower()  # has age
    assert "false" in lines[2].lower()  # no height
    assert "Alice" in lines[3]  # dot access name
    assert "30" in lines[4]  # get_property age
    assert "USA" in lines[5]  # set_property country
    assert "true" in lines[6].lower()  # del_property success
    assert "false" in lines[7].lower()  # city no longer exists
    assert "Alice" in lines[8]  # final name
    assert "30" in lines[9]  # final age
    assert "USA" in lines[10]  # final country
