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


def test_closure_closed_upvalues(run_lox):
    assert (
        run_lox(
            """
    fun outer() {
      var x = "outside";
      fun inner() {
        echo x;
      }

      return inner;
    }

    var closure = outer();
    closure();
    """
        )
        == "outside"
    )


def test_closure_factory_pattern(run_lox):
    assert (
        run_lox(
            """
    fun makeCounter(start) {
        var count = start;
        fun counter() {
            count = count + 1;
            return count;
        }
        return counter;
    }
    
    var counter1 = makeCounter(10);
    var counter2 = makeCounter(100);
    echo counter1();
    echo counter1();
    echo counter2();
    echo counter1();
    """
        )
        == "11\n12\n101\n13"
    )


def test_closure_with_multiple_upvalue_levels(run_lox):
    assert (
        run_lox(
            """
    fun level1() {
        var a = "A";
        fun level2() {
            var b = "B";
            fun level3() {
                var c = "C";
                fun level4() {
                    echo a + b + c;
                }
                return level4;
            }
            return level3();
        }
        return level2();
    }
    
    var deep = level1();
    deep();
    """
        )
        == "ABC"
    )


def test_closure_with_mixed_upvalue_types(run_lox):
    assert (
        run_lox(
            """
    fun outer() {
        var number = 42;
        var text = "hello";
        var flag = true;
        
        fun inner() {
            echo number;
            echo text;
            echo flag;
        }
        
        return inner;
    }
    
    var closure = outer();
    closure();
    """
        )
        == "42\nhello\ntrue"
    )


def test_closure_modifying_multiple_upvalues(run_lox):
    assert (
        run_lox(
            """
    fun calculator() {
        var x = 0;
        var y = 0;
        
        fun setX(value) {
            x = value;
        }
        
        fun setY(value) {
            y = value;
        }
        
        fun add() {
            echo x + y;
        }
        
        setX(10);
        setY(5);
        add();
        setX(20);
        add();
    }
    
    calculator();
    """
        )
        == "15\n25"
    )


def test_closure_chain_with_shadowing(run_lox):
    assert (
        run_lox(
            """
    fun outer() {
        var x = "outer";
        
        fun middle() {
            var x = "middle";
            var y = "from_middle";
            
            fun inner() {
                var x = "inner";
                echo x;        // inner
                echo y;        // from_middle
            }
            
            echo x;            // middle
            return inner;
        }
        
        echo x;                // outer
        return middle();
    }
    
    var innerFunc = outer();
    innerFunc();
    """
        )
        == "outer\nmiddle\ninner\nfrom_middle"
    )


def test_closures_book_25_4_1(run_lox):
    assert (
        run_lox(
            """
            var globalSet;
            var globalGet;
            fun main() {
              var a = "initial";

              fun set() { a = "updated"; }
              fun get() { echo a; }

              globalSet = set;
              globalGet = get;
            }

            main();
            globalSet();
            globalGet();
            """
        )
        == "updated"
    )

def test_closures_book_25_2_2(run_lox):
    assert (
        run_lox(
            """
              fun outer() {
              var x = "value";
              fun middle() {
                fun inner() {
                  echo x;
                }

                echo "create inner closure";
                return inner;
              }

              echo "return from outer";
              return middle;
            }

            var mid = outer();
            var in = mid();
            in();
            """
        )
        == "return from outer\ncreate inner closure\nvalue"
    )
