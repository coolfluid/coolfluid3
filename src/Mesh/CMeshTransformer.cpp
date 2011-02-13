// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>

#include "Common/ComponentPredicates.hpp"
#include "Common/CLink.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/String/Conversion.hpp"

#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CMesh.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::String;

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::CMeshTransformer ( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();
  
  OptionURI::Ptr option;
  option = boost::dynamic_pointer_cast<OptionURI>(m_properties.add_option<OptionURI>("Mesh","The mesh to be transformed",URI()));
  option->supported_protocol(CF::Common::URI::Scheme::CPATH);
  option->attach_trigger ( boost::bind ( &CMeshTransformer::config_mesh,   this ) );
  option->mark_basic();

  m_mesh_link = create_static_component<CLink>("mesh");
  
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::config_mesh()
{
  URI mesh_uri;
  property("Mesh").put_value(mesh_uri);
  CMesh::Ptr mesh = Core::instance().root()->look_component<CMesh>(mesh_uri);
  if ( is_null(mesh) )
    throw CastingFailed (FromHere(), "Mesh must be of a CMesh type");
  set_mesh(mesh);
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::set_mesh(CMesh::Ptr mesh)
{
  m_mesh_link->link_to(mesh);
}

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::~CMeshTransformer()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::execute()
{
  std::vector<std::string> args;
  if (is_null(m_mesh_link->follow()))
    throw BadPointer(FromHere(),"Mesh option is not set");
  if (is_null(m_mesh_link->follow()->as_type<CMesh>()))
    throw CastingFailed (FromHere(), "Mesh must be of a CMesh type");
  transform(m_mesh_link->follow()->as_type<CMesh>());
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::configure_arguments(const std::vector<std::string>& args)
{
  // extract:   variable_name:type=value
  boost::regex expression(  "([[:word:]]+)(\\:([[:word:]]+)(\\[([[:word:]]+)\\])?=(.*))?"  );
  boost::match_results<std::string::const_iterator> what;
  
  boost_foreach (const std::string& arg, args)
  {
    
    std::string name;
    std::string type;
    std::string subtype; // in case of array<type>
    std::string value;
    
    if (regex_search(arg,what,expression))
    {
      name=what[1];
      type=what[3];
      subtype=what[5];
      value=what[6];
      //CFinfo << name << ":" << type << (subtype.empty() ? std::string() : std::string("<"+subtype+">"))  << "=" << value << CFendl;
      if (properties().check(name) == false) // not found
        throw ValueNotFound(FromHere(), "Option with name ["+name+"] not found in "+ full_path().path());      
      
      if (type.empty())
      {
        CFinfo << "this must be a signal without signature" << CFendl;
      }
      else if (type == "bool")
      {
        configure_property(name,from_str<bool>(value));
      }
      else if (type == "unsigned")
      {
        configure_property(name,from_str<Uint>(value));
      }
      else if (type == "integer")
      {
        configure_property(name,from_str<int>(value));
      }
      else if (type == "real")
      {
        configure_property(name,from_str<Real>(value));
      }
      else if (type == "string")
      {
        configure_property(name,value);
      }
      else if (type == "uri")
      {
        configure_property(name,from_str<URI>(value));        
      }
      else if (type == "array")
      {
        std::vector<std::string> array;
        typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
        boost::char_separator<char> sep(",");
        Tokenizer tokens(value, sep);

        for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
        {
          array.push_back(*tok_iter);
          boost::algorithm::trim(array.back()); // remove leading and trailing spaces
        }
        if (subtype == "bool")
        {
          std::vector<bool> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<bool>(str_val));
          configure_property(name,vec);
        }
        else if (subtype == "unsigned")
        {
          std::vector<Uint> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Uint>(str_val));
          configure_property(name,vec);
        }
        else if (subtype == "integer")
        {
          std::vector<int> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<int>(str_val));
          configure_property(name,vec);
        }
        else if (subtype == "real")
        {
          std::vector<Real> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Real>(str_val));
          configure_property(name,vec);
        }
        else if (subtype == "string")
        {
          configure_property(name,array);
        }
        else if (subtype == "uri")
        {
          std::vector<URI> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<URI>(str_val));
          configure_property(name,vec);
        }
      }
      else
      {
        throw ParsingFailed(FromHere(), "The type ["+type+"] of passed argument [" + arg + "] for ["+ full_path().path() +"] is invalid");
      }
    }
    else
      throw ParsingFailed(FromHere(), "Could not parse [" + arg + "] in ["+ full_path().path() +"]");
  }
  
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
