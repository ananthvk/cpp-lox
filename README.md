# cpp-lox

My implementation of second part of Lox programming language from [Crafting interpreters](https://www.craftinginterpreters.com/contents.html) in C++. This implementation uses a bytecode VM to execute the programs.

## How to run?

Install `meson`, `ninja` and a C++ 20 capable compiler

```
$ meson setup builddir
$ cd builddir
$ ninja -j8
```

If you are on linux, run
```
$ ./src/cpplox
```

On windows, run
```
.\src\cpplox.exe
```

## To run tests

To run tests, you need to have `Python 3`

First install `pytest`, a test automation library used to test programs.

Create a virtual environment
```
$ python -m venv .venv
```

Install dependencies
```
$ pip install -r requirements.txt
```

Run tests
```
$ meson setup builddir -Denable-tests=true
$ cd builddir
$ ninja -j8
$ meson test -v
```

## Development build

Have `Clang` installed along with `asan`. You also need to follow the steps in the above section, `pytest` must be installed and available on the `PATH`

Then run the following command,
```
$ ./development.sh
```

## Changes from the book

1) The compiler de-duplicates both strings and integers (but not real numbers)
2) Supports 65k constants, and 65k global variables (accessed with 2 byte unsigned index)

## TODO
- [ ] Fix division by zero error
