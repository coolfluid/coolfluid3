#ifndef FakePE_HH
#define FakePE_HH

namespace CF {
namespace Common {

class FakePE 
{
 public: 
  
  static FakePE& get_instance();
  
  void init();
  
  void finalize();
  
  bool is_init();
  
  int get_rank();
  
  int get_size();
  
  void set_barrier();
  
 private:
  
  FakePE();
  
  ~FakePE();
  
  bool m_is_init;
};

}
}

#endif // FakePE_HH
