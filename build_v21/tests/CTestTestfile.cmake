# CMake generated Testfile for 
# Source directory: C:/Users/vinun/claude AI/tests
# Build directory: C:/Users/vinun/claude AI/build_v21/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(UnitTests "C:/Users/vinun/claude AI/build_v21/tests/test_unit.exe" "--gtest_output=xml:C:/Users/vinun/claude AI/build_v21/results_unit.xml")
set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vinun/claude AI/tests/CMakeLists.txt;28;add_test;C:/Users/vinun/claude AI/tests/CMakeLists.txt;0;")
add_test(IntegrationTests "C:/Users/vinun/claude AI/build_v21/tests/test_integration.exe" "--gtest_output=xml:C:/Users/vinun/claude AI/build_v21/results_integration.xml")
set_tests_properties(IntegrationTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vinun/claude AI/tests/CMakeLists.txt;39;add_test;C:/Users/vinun/claude AI/tests/CMakeLists.txt;0;")
