from subprocess import CalledProcessError


def test_super_access_works(run_lox):
    assert (
        run_lox(
            """
            class A {
              method() {
                println("A method");
              }
            }

            class B : A {
              method() {
                println("B method");
              }

              test() {
                super.method();
              }
            }

            class C : B {}

            C().test();
                   """
        )
        == "A method"
    )


def test_super_closure(run_lox):
    assert (
        run_lox(
            """
        class A {
          method() {
            println("A");
          }
        }

        class B : A {
          method() {
            var closure = super.method;
            closure(); // Prints "A".
          }
        }
        B().method();
        """
        )
        == "A"
    )


def test_super_with_arguments(run_lox):
    assert (
        run_lox(
            """
        class A {
          method(arg) {
            println("A: " + arg);
          }
        }

        class B : A {
          method(arg) {
            super.method(arg);
          }
        }
        B().method("test");
        """
        )
        == "A: test"
    )


def test_super_return_value(run_lox):
    assert (
        run_lox(
            """
        class A {
          getValue() {
            return "from A";
          }
        }

        class B : A {
          getValue() {
            return super.getValue();
          }
        }
        println(B().getValue());
        """
        )
        == "from A"
    )


def test_super_multiple_inheritance_levels(run_lox):
    assert (
        run_lox(
            """
        class A {
          method() {
            println("A");
          }
        }

        class B : A {
          method() {
            println("B");
          }
        }

        class C : B {
          method() {
            super.method();
          }
        }
        C().method();
        """
        )
        == "B"
    )


def test_super_init(run_lox):
    assert (
        run_lox(
            """
        class A {
          init(value) {
            this.value = value;
          }
        }

        class B : A {
          init(value) {
            super.init(value);
            println(this.value);
          }
        }
        B("hello");
        """
        )
        == "hello"
    )


def test_super_method_not_found_error(run_lox):
    try:
        run_lox(
            """
            class A {}

            class B : A {
              test() {
                super.nonexistent();
              }
            }
            B().test();
            """
        )
        assert False, "Expected error for nonexistent super method"
    except CalledProcessError:
        assert True

def test_super_in_nested_class(run_lox):
    assert (
        run_lox(
            """
        class Outer {
          method() {
            println("Outer method");
          }
        }

        class Child : Outer {
          method() {
            class Inner {
              callSuper() {
                // This should still refer to Child's super (Outer)
                return "inner";
              }
            }
            super.method();
            return Inner().callSuper();
          }
        }
        Child().method();
        """
        )
        == "Outer method"
    )


def test_super_without_inheritance_error(run_lox):
    try:
        run_lox(
            """
            class A {
              method() {
                super.something();
              }
            }
            A().method();
            """
        )
        assert False, "Expected error for super in class without inheritance"
    except CalledProcessError:
        pass


def test_super_field_access(run_lox):
    assert (
        run_lox(
            """
        class A {
          init() {
            this.field = "A field";
          }
          
          getField() {
            return this.field;
          }
        }

        class B : A {
          init() {
            super.init();
            this.field = "B field";
          }
          
          getSuperField() {
            return super.getField();
          }
        }
        println(B().getSuperField());
        """
        )
        == "B field"
    )


def test_super_chaining_multiple_levels(run_lox):
    assert (
        run_lox(
            """
        class A {
          method() {
            println("A");
          }
        }

        class B : A {
          method() {
            super.method();
            println("B");
          }
        }

        class C : B {
          method() {
            super.method();
            println("C");
          }
        }
        C().method();
        """
        )
        == "A\nB\nC"
    )


def test_super_this_binding(run_lox):
    assert (
        run_lox(
            """
        class A {
          method() {
            println(this.name);
          }
        }

        class B : A {
          init() {
            this.name = "B instance";
          }
          
          callSuper() {
            super.method();
          }
        }
        B().callSuper();
        """
        )
        == "B instance"
    )
