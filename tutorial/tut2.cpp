// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file tut2.cpp
/// @brief Tutorial 2
///
/// This tutorial shows how to create a simple component "Hello",
/// demonstrating how options and properties are used.

#include "common/Log.hpp"
#include "common/Component.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

/// Component class Hello
class Hello : public Component
{
public:

  /// type name, mandatory to be there!
  static string type_name() { return "Hello"; }

  /// Constructor, with the name of the component as argument
  Hello(const string& name) : Component(name)
  {
    // Add an option person. This is dynamically configurable.
    options().add_option("person", string("nobody"));

    // Add a dynamic property
    properties().add_property("print_count",0);
  }

  /// Destructor, must always be declared virtual
  virtual ~Hello() {}

  /// Print "Hello <person>!", where <person> is configurable.
  void print()
  {
    CFinfo << "Hello " << options()["person"].value<string>() << "!" << CFendl;

    // Increase the count of the times this function is called
    properties()["print_count"]=properties().value<int>("print_count")+1;
  }
};

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  // Allocate new component
  shared_ptr<Hello> hello = allocate_component<Hello>("hello");

  hello->print();
  // output:  Hello nobody!

  hello->options().configure_option("person",string("You"));
  hello->print();
  // output:  Hello You!

  hello->options().configure_option("person",string("World"));
  hello->print();
  // output:  Hello World!

  // Access a property from the component
  CFinfo << "Print is called " << hello->properties().value<int>("print_count") << " times." << CFendl;

  return 0;
}
