from subprocess import CalledProcessError


def test_method_calls_work(run_lox):
    assert (
        run_lox(
            """
        class Foo {
            func1() {
                echo "hello there";
            }
            func2() {
                echo "another function";
            }
        }
        var f = Foo();
        f.func1();
        f.func2();
                   """
        )
        == "hello there\nanother function"
    )


def test_methods_with_parameters(run_lox):
    assert (
        run_lox(
            """
        class Foo {
            noargs() {
                echo "default";
            }
            onearg(x) {
                echo x;
            }
            twoarg(x, y) {
                echo x + y;
            }
        }
        var f = Foo();
        f.noargs();
        f.onearg(3);
        f.twoarg(6, 8);
                   """
        )
        == "default\n3\n14"
    )


def test_methods_bind_to_this(run_lox):
    assert (
        run_lox(
            """
        class Foo {
            fun setup() {
                this.x = 0;
            }
            fun incr() {
                this.x = this.x + 1;
            }
            fun get() {
                return this.x;
            }
        }
        var f = Foo();
        f.setup();
        for(var i = 0; i < 10; i = i + 1) {
            f.incr();
        }
        echo f.get();
                   """
        )
        == "10"
    )


def test_method_not_found(run_lox):
    try:
        run_lox(
            """
        class Foo {
            fun setup() {
                this.x = 0;
            }
            fun incr() {
                this.x = this.x + 1;
            }
            fun get() {
                return this.x;
            }
        }
        var f = Foo();
        f.dosomethingelse();
                   """
        )
        assert False
    except CalledProcessError:
        assert True


def test_reassign_methods_dynamically(run_lox):
    assert (
        run_lox(
            """
        class Counter {
            fun setup() {
                this.x = 0;
            }
            fun incr() {
                this.x = this.x + 1;
            }
            fun get() {
                return this.x;
            }
        }
        var f = Counter();
        f.setup();
        var tmp = f.incr;
        f.increment = tmp;
        for(var i = 0; i < 10; i = i + 1) {
            f.increment();
        }
        echo f.get();
                   """
        )
        == "10"
    )


def test_init_method(run_lox):
    assert (
        run_lox(
            """
        class Counter {
            fun init() {
                this.x = 0;
            }
            fun incr() {
                this.x = this.x + 1;
            }
        }
        var f = Counter();
        f.incr();
        f.incr();
        f.incr();
        echo f.x;
                   """
        )
        == "3"
    )


def test_init_method_with_parameters(run_lox):
    assert (
        run_lox(
            """
        class Counter {
            fun init(value) {
                this.x = value;
            }
            fun incr() {
                this.x = this.x + 1;
            }
        }
        var f = Counter(30);

        // check if init method gets called automatically
        echo f.x;
        
        // Setting value on instance
        f.x = 100;
        echo f.x;
        
        // Calling f on the instance should return back the same instance
        var g = f.init(99);
        echo g == f;
        echo f.x;
        echo g.x;

                   """
        )
        == "30\n100\ntrue\n99\n99"
    )


def test_init_method_return_value_should_fail(run_lox):
    try:
        run_lox(
            """
            class Counter {
                fun init() {
                    this.x = 0;
                    return "some string";
                }
                fun incr() {
                    this.x = this.x + 1;
                }
            }
            var f = Counter();
            f.incr();
            f.incr();
            f.incr();
            echo f.x;
                       """
        )
        assert False
    except CalledProcessError:
        assert True


def test_init_method_return_no_value_should_pass(run_lox):
    assert (
        run_lox(
            """
            class Counter {
                fun init(val) {
                    if(val > 10) {
                        this.x = 100;
                        return;
                    } else {
                        this.x = val;
                        return;
                    }
                }
                fun incr() {
                    this.x = this.x + 1;
                }
            }
            var f = Counter(5);
            f.incr();
            echo f.x;
            f.init(80);
            echo f.x;

                       """
        )
        == "6\n100"
    )


def test_methods_bound_to_instance(run_lox):
    assert (
        run_lox(
            """
            class Person {
                setup() {
                    this.counter = 0;
                }
                increment(val) {
                    this.counter = this.counter + val;
                }
                display() {
                    echo this.counter;
                }
            }

            const p1 = Person();
            const p2 = Person();

            p1.setup();
            p2.setup();

            // Swap bound methods
            var tmp = p1.increment;
            p1.increment = p2.increment;
            p2.increment = tmp;

            // Call them
            p1.increment(10);
            p2.increment(5);

            p1.display();
            p2.display();
            """
        )
        == "5\n10"
    )


def test_inner_function_capture_this(run_lox):
    assert (
        run_lox(
            """
            class Foo {
                init(count) {
                    this.counter = count;
                }
                display() {
                    fun inner() {
                        echo this.counter;
                    }
                    return inner;
                }
                counter_incrementer() {
                    fun nested_fun() {
                        fun inner() {
                            this.counter = this.counter + 1;
                        }
                        return inner;
                    }
                    return nested_fun;;
                }
            }

            const f = Foo(10);
            var display_fn = f.display();
            var incr = f.counter_incrementer()();
            incr();
            display_fn();
            incr();
            display_fn();
            incr();
            incr();
            display_fn();
            f.counter = 0;
            display_fn();
            """
        )
        == "11\n12\n14\n0"
    )


def test_field_method_name_collision(run_lox):
    assert (
        run_lox(
            """
            class Foo {
                init() {
                    this.bar = "field value";
                }
                bar() {
                    return "method value";
                }
            }

            const f = Foo();
            echo f.bar;        // Should access field, not method
            """
        )
        == "field value"
    )


def this_outside_class(run_lox):
    try:
        run_lox(
            """
        class Foo {
            fun setup() {
                this.x = 0;
            }
            fun incr() {
                this.x = this.x + 1;
            }
            fun get() {
                return this.x;
            }
        }
        var f = Foo();
        echo this;
                   """
        )
        assert False
    except CalledProcessError:
        assert True


def test_nested_classes(run_lox):
    assert (
        run_lox(
            """
        class Outer {
            getcls() {
                var inst = this;
                inst.x = 32;
                class Inner {
                    init() {
                        this.x = inst.x;
                    }
                    display () {
                        echo this.x;
                    }
                    increment() {
                        this.x = this.x + 1;
                        inst.x = inst.x + 10;
                    }
                }
                return Inner;
            }
        }

        var out = Outer();
        var inner_class = out.getcls();
        var instance = inner_class();

        instance.increment();
        instance.display();
        echo out.x;
        """
        )
        == "33\n42"
    )
