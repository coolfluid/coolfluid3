# Test whether the compiler supports explicit template instantiation.
# This actually creates a class template instantiation in one source
# file and tries to use it from another.  This approach checks that
# both the instantiation syntax and symbol linkage is handled
# properly.

file( MAKE_DIRECTORY ${coolfluid_BINARY_DIR}/CMakeTmp/CheckExplicitInstantiation)
string(ASCII 35 POUND)
 write_file(
   ${coolfluid_BINARY_DIR}/CMakeTmp/CheckExplicitInstantiation/CMakeLists.txt
   "PROJECT(EXPLICIT)\n"
   "ADD_LIBRARY(A A.cxx)\n"
   "add_executable(B B.cxx)\n"
   "TARGET_LINK_LIBRARIES(B A)\n"
   )
 write_file(
   ${coolfluid_BINARY_DIR}/CMakeTmp/CheckExplicitInstantiation/A.h
   "${POUND}ifndef A_h\n"
   "${POUND}define A_h\n"
   "template <class T> class A { public: static T Method(); };\n"
   "${POUND}endif\n"
   )
 write_file(
   ${coolfluid_BINARY_DIR}/CMakeTmp/CheckExplicitInstantiation/A.cxx
   "${POUND}include \"A.h\"\n"
   "template <class T> T A<T>::Method() { return 0; }\n"
   "template class A<int>;"
   )
 write_file(
   ${coolfluid_BINARY_DIR}/CMakeTmp/CheckExplicitInstantiation/B.cxx
   "${POUND}include \"A.h\"\n"
   "int main() { return A<int>::Method(); }\n"
   )
 TRY_COMPILE( CF3_CXX_SUPPORTS_EXPLICIT_TEMPLATES
   ${coolfluid_BINARY_DIR}/CMakeTmp/CheckExplicitInstantiation/Build
   ${coolfluid_BINARY_DIR}/CMakeTmp/CheckExplicitInstantiation
   EXPLICIT OUTPUT_VARIABLE OUTPUT
   )

 if(CF3_CXX_SUPPORTS_EXPLICIT_TEMPLATES)
   set(CF3_CXX_SUPPORTS_EXPLICIT_TEMPLATES ON CACHE INTERNAL "Support for C++ explict templates")
   coolfluid_log_file( "Determining if the C++ compiler supports explict template instantiation passed with the following output:\n ${OUTPUT}\n" )
 else()
   set(CF3_CXX_SUPPORTS_EXPLICIT_TEMPLATES OFF CACHE INTERNAL "Support for C++ explict templates")
   coolfluid_log_file( "Determining if the C++ compiler supports explict template instantiation failed with the following output:\n ${OUTPUT}\n" )
 endif()

coolfluid_log_file( "+++++  Checking support for C++ explicit template instantiation -- ${CF3_CXX_SUPPORTS_EXPLICIT_TEMPLATES}")


