// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "Common/Foreach.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/OptionList.hpp"
#include "Common/XML/Protocol.hpp"
#include "Common/OptionArray.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

//Option::Ptr OptionList::add_property (const std::string& name,
//                                          const boost::any & value )
//{
//  cf_assert_desc ( "Class has already property with same name",
//                   this->store.find(name) == store.end() );
//  //
//  // cf_assert_desc("The name of property ["+name+"] does not comply with coolfluid standard. "
//  //                "It may not contain spaces.",
//  //   boost::algorithm::all(name,
//  //     boost::algorithm::is_alnum() || boost::algorithm::is_any_of("-_")) );

//  Property::Ptr prop ( new Property(value) );
//  store.insert( std::make_pair(name, prop ) );
//  return prop;
//}

////////////////////////////////////////////////////////////////////////////////

const Option & OptionList::option( const std::string& pname) const
{
  OptionStorage_t::const_iterator itr = store.find(pname);

  if ( itr != store.end() )
    return *itr->second.get();
  else
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available properties are:\n";
    OptionStorage_t::const_iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }

}

////////////////////////////////////////////////////////////////////////////////

Option & OptionList::option( const std::string& pname)
{
  OptionStorage_t::iterator itr = store.find(pname);

  if ( itr != store.end() )
    return *itr->second.get();
  else
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available properties are:\n";
    OptionStorage_t::iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }

}

////////////////////////////////////////////////////////////////////////////////

//const Option & OptionList::option( const std::string& pname) const
//{
//  return option(pname).as_option();
//}
//////////////////////////////////////////////////////////////////////////////////

//Option & OptionList::option( const std::string& pname)
//{
//  return option(pname).as_option();
//}

////////////////////////////////////////////////////////////////////////////////

void OptionList::erase( const std::string& pname)
{
  OptionStorage_t::iterator itr = store.find(pname);
  if ( itr != store.end() )
    store.erase(itr);
  else
    throw ValueNotFound(FromHere(), "Option with name [" + pname + "] not found" );
}

////////////////////////////////////////////////////////////////////////////////

Option & OptionList::operator [] (const std::string & pname)
{
  Option::Ptr opt;
  OptionStorage_t::iterator itr = store.find(pname);

  if ( itr != store.end() )
    opt = itr->second;
  else
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available options are:\n";
    OptionStorage_t::const_iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }

  return *opt.get();
}

////////////////////////////////////////////////////////////////////////////////

const Option & OptionList::operator [] (const std::string & pname) const
{
  Option::ConstPtr opt;
  OptionStorage_t::const_iterator itr = store.find(pname);

  if ( itr != store.end() )
    opt = itr->second;
  else
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available options are:\n";
    OptionStorage_t::const_iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }
  return *opt.get();
}

////////////////////////////////////////////////////////////////////////////////

void OptionList::configure_option(const std::string& pname, const boost::any& val)
{
  OptionStorage_t::iterator itr = store.find(pname);
  if (itr == store.end())
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available options are:\n";
    if (store.size())
    {
      OptionStorage_t::iterator it = store.begin();
      for (; it!=store.end(); it++)
        msg += "  - " + it->first + "\n";
    }
    throw ValueNotFound(FromHere(),msg);
  }
  Option::Ptr prop = itr->second;

  // update the value and trigger its actions
  prop->change_value(val);
}

////////////////////////////////////////////////////////////////////////////////

std::string OptionList::list_options()
{
  std::string opt_list="";
//  Uint cnt(0);
//  foreach_container( (const std::string& name) (Property::Ptr property) , *this )
//  {
//    if ( Option::Ptr option = boost::dynamic_pointer_cast<Option>(property) )
//    {
//      if (cnt > 0)
//        opt_list=opt_list+"\n";

//      if (option->tag() ==  XML::Protocol::Tags::node_array())
//      {
//        OptionArray::Ptr array_option = boost::dynamic_pointer_cast<OptionArray>(option);
//        std::string values=array_option->value_str();
//        boost::algorithm::replace_all(values, "@@", ",");
//        opt_list = opt_list+name+":array["+array_option->elem_type()+"]="+values;
//      }
//      else
//      {
//        opt_list = opt_list+name+":"+option->type()+"="+option->value_str();
//      }
//      ++cnt;
//    }
//  }
  return opt_list;
}

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

