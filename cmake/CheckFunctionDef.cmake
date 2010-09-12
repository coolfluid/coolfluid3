# try to compile a program
check_cxx_source_compiles(
" #include <iostream>
  int main(int argc, char* argv[])
  {
    std::cout << __FUNCTION__ << std::endl;
    return 0;
  }"
  CF_HAVE_FUNCTION_DEF )
