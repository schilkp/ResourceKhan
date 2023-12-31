configure:
    -rm -rf build
    mkdir build
    cd build && cmake .. -DCMAKE_GENERATOR=Ninja

build:
    cd build && ninja

clean:
    cd build && ninja clean

deepclean:
    -rm -rf build

test: build
    python scripts/run_tests.py build/bin/

format:
    python scripts/run_clang_format.py
