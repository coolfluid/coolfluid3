// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_FaceLoop_hpp
#define CF_RDM_FaceLoop_hpp

#include "RDM/Core/ElementLoop.hpp"

#include "RDM/Core/SupportedFaces.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/// FaceLoop defines the base class for all FaceLoopT
/// It is the core of the looping mechanism over Faces.
struct FaceLoop : public ElementLoop
{
  /// Constructor
  FaceLoop( const std::string& name ) : ElementLoop(name) {  regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "FaceLoop"; }

}; // FaceLoop

/// FaceLoopT defines a functor taking the type that boost::mpl::for_each passes.
template < typename ACTION, typename PHYS>
struct FaceLoopT : public FaceLoop
{
  /// Constructor
  FaceLoopT( const std::string& name ) : FaceLoop(name) {  regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "FaceLoopT<" + ACTION::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ()
  {
    boost::mpl::for_each< typename RDM::FaceTypes< PHYS::ndim >::Faces >( boost::ref(*this) );
  }

  /// operator needed for the loop over element types (SF)
  template < typename SF >
  void operator() ( SF& )
  {
    if( is_null(parent().as_ptr<ACTION>()) )
      throw Common::SetupError(FromHere(), type_name() + " was intantiated with wrong action");

    /// definition of the quadrature type
    typedef typename RDM::DefaultQuadrature<SF>::type QD;
    /// parametrization of the numerical term
    typedef typename ACTION::template Term< SF, QD, PHYS > TermT;

    // loop on the (sub)regions that hold elements of this type

    boost_foreach(Mesh::CElements& elements,
                  Common::find_components_recursively_with_filter<Mesh::CElements>(*current_region,IsElementType<SF>()))
    {
      TermT& term = this->access_term<TermT>();

      // point the term to the elements of the (sub)region
      term.set_elements(elements);

      const Uint nb_elem = elements.size();
      for ( Uint elem = 0; elem != nb_elem; ++elem )
      {
        term.select_loop_idx(elem);
        term.executeT();
      }
    }
  }

}; // FaceLoopT

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_FaceLoop_hpp
