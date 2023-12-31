import os
import subprocess
import sys
from collections import defaultdict
from os.path import basename, isfile, join, splitext


class Color:
    OK = '\033[92m'
    WARN = '\033[93m'
    ERR = '\033[91m'
    INFO = '\033[96m'
    END = '\33[0m'


def main(test_dir):

    test_suites = []

    for file in os.listdir(test_dir):
        if file.endswith(".test"):
            continue
        test = join(test_dir, file)
        if isfile(test):
            test_suites.append(test)

    # Print message and exit if there where no tests passed:
    if len(test_suites) == 0:
        print("No test suites found.")
        sys.exit(0)

    # Empty line to clean up makefile output a bit.
    print()

    tests_crashed = []
    tests_passed = defaultdict(list)
    tests_failed = defaultdict(list)
    tests_ignored = defaultdict(list)

    # Run every test
    for test_suite in test_suites:
        test_suite_name = splitext(basename(test_suite))[0]
        print("Running test suite %s..." % test_suite_name)

        try:
            test_output = subprocess.run(
                test_suite, capture_output=True, timeout=0.10)
        except subprocess.TimeoutExpired:
            print(Color.ERR, end='')
            print("Test suite %s timed-out, test results not reported!" %
                  test_suite_name)
            print(Color.END, end='')
            tests_crashed.append(test_suite_name)
            continue

        suite_output = test_output.stdout.decode()

        suite_fail_count = 0

        if '-----------------------' not in suite_output:
            # Test suite crashed
            print(Color.ERR, end='')
            print("Test suite %s crashed, test results not reported!" %
                  test_suite_name)
            print(Color.END, end='')
            tests_crashed.append(test_suite_name)
        else:
            # Test suite completed

            # Save output to .test file:
            with open(splitext(test_suite)[0] + '.test', 'w') as report:
                report.write(suite_output)

            # Separate and sort tests:
            for line in suite_output.split('\n'):
                if line == '' or line == '-----------------------':
                    # Report finished
                    break
                elif ':PASS' in line:
                    tests_passed[test_suite].append(cleanup_test_result(line))
                elif ':FAIL' in line:
                    tests_failed[test_suite].append(cleanup_test_result(line))
                    suite_fail_count += 1
                elif ':IGNORE' in line:
                    tests_ignored[test_suite].append(cleanup_test_result(line))
                elif ':INFO' in line:
                    print(Color.INFO, end='')
                    print(line)
                    print(Color.END, end='')
                else:
                    print(Color.WARN, end='')
                    print("Error parsing test output '%s', ignoring line. Did the test crash?"
                          % line)
                    print(Color.END, end='')

        # Unity test suites return the number of failed tests.
        # Make sure that matches what we parsed above:
        failed_tests_unity_report = test_output.returncode

        if suite_fail_count != failed_tests_unity_report:
            print(Color.WARN, end='')
            print("Unity report %i failed tests, but %i where tracked by the test output." % (
                failed_tests_unity_report, suite_fail_count))
            print("Verify test suite output manually!")
            print(Color.END, end='')

    # Print summary:
    crashes = len(tests_crashed)
    passes = len([len(l) for l in tests_passed.values()])
    fails = len([len(l) for l in tests_failed.values()])
    ignores = len([len(l) for l in tests_ignored.values()])
    tests = passes + fails + ignores

    print()
    print()
    print("============= Summary =============")

    if crashes != 0:
        print(Color.ERR, end='')
        print("Warning! %i test suite(s) crashed. Not all tests were performed!" % crashes)
        print(Color.END, end='')

    print("Ran %i tests." % tests)

    print(Color.ERR, end='')
    print("Failed: %i" % fails)
    print(Color.END + Color.OK, end='')
    print("Passed: %i" % passes)
    print(Color.END + Color.WARN, end='')
    print("Ignore: %i" % ignores)
    print(Color.END, end='')

    if (tests - ignores) != 0:
        print("Success rate (without ignored tests): %2.2f%%" %
              (float(passes) / (tests - ignores) * 100))

    print()
    print("============ Breakdown ============")

    if passes != 0:
        print("Passed (%i): " % passes)
        for suite in sorted(tests_passed):
            print(Color.OK, end='')
            print(f"  {basename(suite)}:")
            print(Color.END, end='')
            for test in tests_passed[suite]:
                print(f"   - {test}")
        print()

    if ignores != 0:
        print("Ignored (%i): " % ignores)
        for suite in sorted(tests_ignored):
            print(Color.WARN, end='')
            print(f"  {basename(suite)}:")
            print(Color.END, end='')
            for test in tests_ignored[suite]:
                print(f"   - {test}")
        print()

    if fails != 0:
        print("Failed (%i): " % fails)
        for suite in sorted(tests_failed):
            print(Color.ERR, end='')
            print(f"  {basename(suite)}:")
            print(Color.END, end='')
            for test in tests_failed[suite]:
                print(f"   - {test}")
        print()

    if crashes != 0:
        print("Crashed test suites (%i):" % crashes)
        print(Color.ERR, end='')
        for suite in sorted(tests_crashed):
            print(f"  {basename(suite)}")
        print(Color.END, end='')
        print()

    print("===================================")
    if crashes == 0 and fails == 0:
        print(Color.OK, end='')
        print("All good! :)")
        print(Color.END)
        print()
        print()
        sys.exit(0)
    else:
        print(Color.WARN, end='')
        print("There is some work left to do...")
        print(Color.END, end='')
        print()
        print()
        sys.exit(1)


def cleanup_test_result(line) -> str:
    # Unity test results are of the form:
    #   SOURCE_NAME:LINE_NO:TEST_NAME:RESULT
    #
    # We strip the path from SOURCE_NAME to make the output a bit less spammy.
    parts = line.split(":")
    parts[0] = basename(parts[0])
    return ":".join(parts)


if __name__ == '__main__':
    args = sys.argv.copy()
    path = args.pop(0)
    if len(args) != 1:
        print("Error! Must specify directory with test binaries as only argument!")
        exit(1)
    main(args[0])
