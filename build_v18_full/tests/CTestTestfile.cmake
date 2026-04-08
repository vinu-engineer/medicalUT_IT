# CMake generated Testfile for 
# Source directory: C:/Users/vinun/claude AI/tests
# Build directory: C:/Users/vinun/claude AI/build_v18_full/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(UnitTests "C:/Users/vinun/claude AI/build_v18_full/tests/test_unit.exe" "--gtest_output=xml:C:/Users/vinun/claude AI/build_v18_full/results_unit.xml")
set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vinun/claude AI/tests/CMakeLists.txt;25;add_test;C:/Users/vinun/claude AI/tests/CMakeLists.txt;0;")
add_test(IntegrationTests "C:/Users/vinun/claude AI/build_v18_full/tests/test_integration.exe" "--gtest_output=xml:C:/Users/vinun/claude AI/build_v18_full/results_integration.xml")
set_tests_properties(IntegrationTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vinun/claude AI/tests/CMakeLists.txt;36;add_test;C:/Users/vinun/claude AI/tests/CMakeLists.txt;0;")
