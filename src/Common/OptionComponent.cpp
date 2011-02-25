// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// #include <boost/foreach.hpp>
// 
#include "Common/OptionComponent.hpp"
// #include "Common/XmlHelpers.hpp"
// #include "Common/CRoot.hpp"
// #include "Common/Core.hpp"
// #include "Common/Log.hpp"

using namespace CF::Common;
// 
// OptionComponent::OptionComponent(const std::string & name, const std::string & desc,
//                      const URI & def) :
//   Option(name, desc, linked_type(Core::instance().root()->access_component_ptr(def)))
// {
//   TypeInfo::instance().regist<linked_type>("value_type");
//   
//   CFLogVar(class_name<linked_type>());
//   CFLogVar(type());
// }
// 
// ///////////////////////////////////////////////////////////////////////////////
// 
// OptionComponent::~OptionComponent()
// {
// 
// }
// 
// ///////////////////////////////////////////////////////////////////////////////
// 
// void OptionComponent::configure ( XmlNode& node )
// {
//   URI val;
//   XmlNode * type_node = node.first_node(XmlTag<URI>::type());
// 
//   if(type_node != nullptr)
//     to_value(*type_node, val);
//   else
//     throw XmlError(FromHere(), "Could not find a value for this option.");
// 
//   m_value = linked_type(Core::instance().root()->access_component_ptr(val));
// }
// 
// //////////////////////////////////////////////////////////////////////////////
// 
// void OptionComponent::copy_to_linked_params ( const boost::any& val )
// {
//   CF_DEBUG_POINT;
//   BOOST_FOREACH ( void* v, this->m_linked_params )
//   {
//     CF_DEBUG_POINT;
//     
//     linked_type* cv = static_cast<linked_type*>(v);
//     CF_DEBUG_POINT;
//     
//     *cv = linked_type(Core::instance().root()->access_component_ptr(boost::any_cast<URI>(val)));
//   }
//   CF_DEBUG_POINT;
//   
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// std::string OptionComponent::value_str () const
// { 
//   CF_DEBUG_POINT;
//   
//   linked_type val = boost::any_cast< linked_type >(m_value);
//   
//   CF_DEBUG_POINT;
//   
//   if (is_null(val.lock()))
//     throw ValueNotFound(FromHere(),"option_value invalid");
//   
//   CF_DEBUG_POINT;
//   
//   return val.lock()->full_path().string(); 
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// /// @returns the default value as a sd::string
// std::string OptionComponent::def_str () const
// { 
//   return from_value( def<URI>() ); 
// }

//////////////////////////////////////////////////////////////////////////////

