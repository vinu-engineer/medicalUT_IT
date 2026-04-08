# CMake generated Testfile for 
# Source directory: C:/Users/vinun/claude AI/tests
# Build directory: C:/Users/vinun/claude AI/build_v23/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(UnitTests "C:/Users/vinun/claude AI/build_v23/tests/test_unit.exe" "--gtest_output=xml:C:/Users/vinun/claude AI/build_v23/results_unit.xml")
set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vinun/claude AI/tests/CMakeLists.txt;29;add_test;C:/Users/vinun/claude AI/tests/CMakeLists.txt;0;")
add_test(IntegrationTests "C:/Users/vinun/claude AI/build_v23/tests/test_integration.exe" "--gtest_output=xml:C:/Users/vinun/claude AI/build_v23/results_integration.xml")
set_tests_properties(IntegrationTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vinun/claude AI/tests/CMakeLists.txt;40;add_test;C:/Users/vinun/claude AI/tests/CMakeLists.txt;0;")
