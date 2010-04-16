#include "Common/Log.hpp"
#include "Common/Component.hpp"

#include <iostream>
#include <map>
#include <string>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/dynamic_property_map.hpp>

using namespace CF;
using namespace CF::Common;

template <typename AddressMap>
void foo(AddressMap address)
{
  typedef typename boost::property_traits<AddressMap>::value_type value_type;
  typedef typename boost::property_traits<AddressMap>::key_type key_type;

  value_type old_address, new_address;
  key_type fred = "Fred";
  old_address = get(address, fred);
  new_address = "384 Fitzpatrick Street";
  put(address, fred, new_address);

  key_type joe = "Joe";
  value_type& joes_address = address[joe];
  joes_address = "325 Cushing Avenue";
}

int main(int argc, char * argv[])
{

  //  CFinfo << "Welcome to the COOLFLUID K3 solver!\n" << CFendl;

  CFinfo << "----------------------------------------------------!\n" << CFendl;

  std::map<std::string, std::string> name2address;
  boost::associative_property_map< std::map<std::string, std::string> >
    address_map(name2address);

  name2address.insert(make_pair(std::string("Fred"), std::string("710 West 13th Street")));
  name2address.insert(make_pair(std::string("Joe"),  std::string("710 West 13th Street")));

  foo(address_map);
  
  for (std::map<std::string, std::string>::iterator i = name2address.begin();
       i != name2address.end(); ++i)
    std::cout << i->first << ": " << i->second << "\n";

  CFinfo << "----------------------------------------------------!\n" << CFendl;

  // build property maps using associative_property_map
  std::map<std::string, int> name2age;
  std::map<std::string, double> name2gpa;
  boost::associative_property_map< std::map<std::string, int> >
    age_map(name2age);
  boost::associative_property_map< std::map<std::string, double> >
    gpa_map(name2gpa);

  std::string fred("Fred");
  // add key-value information
  name2age.insert(make_pair(fred,17));
  name2gpa.insert(make_pair(fred,2.7));

  // build and populate dynamic interface
  boost::dynamic_properties properties;
  properties.property("age",age_map);
  properties.property("gpa",gpa_map);

  std::string mark = "Mark";

  int old_age = boost::get<int>("age", properties, mark);
  std::string old_gpa = boost::get("gpa", properties, mark);

  std::cout << "Mark's old age: " << old_age << "\n"
            << "Mark's old gpa: " << old_gpa << "\n";

  std::string new_age = "18";
  double new_gpa = 3.9;
  boost::put("age",properties,mark,new_age);
  boost::put("gpa",properties,mark,new_gpa);

  std::cout << "Mark's age: " << boost::get(age_map,mark) << "\n"
            << "Mark's gpa: " << boost::get(gpa_map,mark) << "\n";

  CFinfo << "----------------------------------------------------!\n" << CFendl;



  return EXIT_SUCCESS;
}
