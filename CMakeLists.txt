cmake_minimum_required(VERSION 3.0.0)
project(aesd-assignments)
# A list of all automated test source files
# At minimum it should include the files in the test/assignmentX directory
# Students can also add paths to their own test files using a ../ prefix to
# reference this working directory

set(CMAKE_C_FLAGS "-pthread")

set(AUTOTEST_SOURCES
    test/assignment1/Test_hello.c
    test/assignment1/Test_assignment_validate.c
<<<<<<< HEAD
=======

    ../student-test/assignment1/Test_validate_username.c

    test/assignment3/Test_systemcalls.c


    test/assignment4/Test_threading.c

>>>>>>> 72adb1697ec689eb5a5a406095e49ce27d6ecb74
)
# A list of all files containing test code that is used for assignment validation
set(TESTED_SOURCE
    ../examples/autotest-validate/autotest-validate.c
)
