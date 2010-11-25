// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Common_BuildComponent_hpp
#define CF_Common_BuildComponent_hpp

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

const int full       = 0x0;
const int noprops    = 0x1;
const int nosignals  = 0x2;
const int none       = noprops | nosignals;

////////////////////////////////////////////////////////////////////////////////

template < int mode = full >
struct BuildComponent
{
  template < typename TYPE >
  void build ( TYPE* self );
};

template <>
template < typename TYPE >
void BuildComponent< full >::build( TYPE* self )
{
  self->add_tag( TYPE::type_name() );
  self->define_config_properties();
  self->define_signals();
}

template <>
template < typename TYPE >
void BuildComponent< noprops >::build( TYPE* self )
{
  self->add_tag( TYPE::type_name() );
  self->define_signals();
}

template <>
template < typename TYPE >
void BuildComponent< nosignals >::build( TYPE* self )
{
  self->add_tag( TYPE::type_name() );
  self->define_config_properties();
}

template <>
template < typename TYPE >
void BuildComponent< none >::build( TYPE* self )
{
  self->add_tag( TYPE::type_name() );
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#endif // CF_Common_BuildComponent_hpp
