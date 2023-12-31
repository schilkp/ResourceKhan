_default:
    just --list

# Configure the build system.
configure:
    mkdir -p build
    cd build && cmake .. -DCMAKE_GENERATOR=Ninja

# Compile.
build:
    cd build && ninja

# Clean compilation artifacts.
clean:
    cd build && ninja clean

# Clean compilation artifacts, build system artifacts, and dependencies.
deepclean:
    -rm -rf build

# Compile and run tests.
test: build
    python scripts/run_tests.py build/bin/

# Auto-format all code.
format:
    python scripts/run_clang_format.py
