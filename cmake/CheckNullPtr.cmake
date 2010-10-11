# check if the compiler supports the nullptr idiom
# See Scott Meyer's "Exceptional C++"

set( CHECK_nullptr_SRC "
const class nullptr_t
{
  public:
    template<class T>
      operator T*() const { return 0; }

    template<class C, class T>
    operator T C::*() const { return 0; }

  private:
    void operator&() const;

} nullptr = {};

struct C
{
  void func();
};

template<typename T>
void g( T* t ) {}

template<typename T>
void h( T t ) {}

void func (double *) {}
void func (int) {}

int main(void)
{
  char * ch = nullptr;        // ok
  func (nullptr);             // Calls func(double *)
  func (0);                   // Calls func(int)
  void (C::*pmf2)() = 0;      // ok
  void (C::*pmf)() = nullptr; // ok
  nullptr_t n1, n2;
  n1 = n2;

  if (nullptr == ch) {}       // ok
}
")

check_cxx_source_compiles("${CHECK_nullptr_SRC}" CF_CXX_SUPPORTS_NULLPTR )

# coolfluid_log( "CF_CXX_SUPPORTS_NULLPTR [${CF_CXX_SUPPORTS_NULLPTR}]" )
