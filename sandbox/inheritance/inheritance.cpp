#include <iostream>
#include <typeinfo>

//------------------------------------------------------------------------------------------

template < typename TCHILD, typename TDerive, typename TLIB >
struct Derive : public TDerive {
public:

  typedef TDerive PARENT;
  typedef TLIB  LIB;

  Derive( const std::string& name ) : TDerive(name) {}

}; // !Derive

//------------------------------------------------------------------------------------------

struct LibCommon {
    static std::string library_namespace() { return "cf3.common"; }
};

struct LibCore {
    static std::string library_namespace() { return "cf3.common.Core"; }
};

//------------------------------------------------------------------------------------------

class Component {
public:

  typedef LibCommon LIB; /* component is the only one that needs this here */

  Component( const std::string& name ) : m_name(name) {}

  virtual ~Component() {}

  std::string name() const { return m_name; };

  virtual void pure_virtual_function() = 0;
  virtual void virtual_function()
    { std::cout << name() << " -> Component::virtual_function()" << std::endl; };

private:
  std::string m_name;
};

//------------------------------------------------------------------------------------------

struct B : public Derive< B, Component, LibCommon >
{
  B( const std::string& name ) : Derive<B,Component,LibCommon>(name) {}

  virtual ~B() {}

  virtual void pure_virtual_function()
    { std::cout << name() << " -> B::pure_virtual_function()" << std::endl; };
};

//------------------------------------------------------------------------------------------

struct A : public Derive< A, B, LibCore >
{
  A( const std::string& name ) : Derive<A,B,LibCore>(name) {}

  virtual ~A() {}

  virtual void virtual_function()
    { std::cout << name() << " -> A::virtual_function()" << std::endl; };
};

//------------------------------------------------------------------------------------------


int main(void)
{
  std::cout << "Component  :" << typeid( Component ).name() << std::endl;
  std::cout << "B::PARENT  :" << typeid( B::PARENT ).name() << std::endl;
  std::cout << "B          :" << typeid( B ).name() << std::endl;
  std::cout << "A::PARENT  :" << typeid( A::PARENT ).name() << std::endl;
  std::cout << "A::PARENT::PARENT  :" << typeid( A::PARENT::PARENT ).name() << std::endl;
  std::cout << "A          :" << typeid( A ).name() << std::endl;

  std::cout << "------------------" << std::endl;

  std::cout << "A::LIB                   :" << A::LIB::library_namespace() << std::endl;
  std::cout << "A::PARENT::LIB           :" << A::PARENT::LIB::library_namespace() << std::endl;
  std::cout << "A::PARENT::PARENT::LIB   :" << A::PARENT::PARENT::LIB::library_namespace() << std::endl;

  std::cout << "------------------" << std::endl;

  Component* c = new A( "lolo" );

  c->virtual_function();
  c->pure_virtual_function();

  return 0;
}
