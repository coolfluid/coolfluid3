#include <boost/assign/list_of.hpp> // for map_list_of

#include "GUI/Client/OptionType.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::GUI::Client;

////////////////////////////////////////////////////////////////////////////////

OptionType::Convert::FwdMap_t OptionType::Convert::all_fwd = boost::assign::map_list_of
( OptionType::INVALID,           "INVALID" )
( OptionType::TYPE_BOOL,         "bool")
( OptionType::TYPE_INT,          "integer")
( OptionType::TYPE_UNSIGNED_INT, "unsigned")
( OptionType::TYPE_DOUBLE,       "real" )
( OptionType::TYPE_STRING,       "string")
( OptionType::TYPE_FILES,        "files")
( OptionType::TYPE_LIBRARIES,    "libraries")
( OptionType::TYPE_ARRAY,        "array")
( OptionType::TYPE_PATH,         "file");

OptionType::Convert::BwdMap_t OptionType::Convert::all_rev = boost::assign::map_list_of
( "INVALID",   OptionType::INVALID )
( "bool",      OptionType::TYPE_BOOL )
( "integer",   OptionType::TYPE_INT)
( "unsigned",  OptionType::TYPE_UNSIGNED_INT )
( "real",      OptionType::TYPE_DOUBLE )
( "string",    OptionType::TYPE_STRING )
( "files",     OptionType::TYPE_FILES )
( "libraries", OptionType::TYPE_LIBRARIES )
( "array",     OptionType::TYPE_ARRAY )
( "file",      OptionType::TYPE_PATH );

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const OptionType::Type& in )
{
  os << OptionType::Convert::to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, OptionType::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = OptionType::Convert::to_enum(tmp);
  return is;
}


////////////////////////////////////////////////////////////////////////////////
