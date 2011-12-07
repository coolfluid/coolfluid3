// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_CellLoop_hpp
#define cf3_RDM_CellLoop_hpp

#include "mesh/Field.hpp"

#include "RDM/ElementLoop.hpp"
#include "RDM/SupportedCells.hpp"
#include "RDM/CellTerm.hpp"

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////


/// CellLoop defines the base class for all CellLoopT
/// It is the core of the looping mechanism over Cells
struct CellLoop : public ElementLoop
{
  /// Constructor
  CellLoop( const std::string& name ) : ElementLoop(name) {  regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "CellLoop"; }

  /// Access the term
  /// Will create it if does not exist.
  /// @return reference to the term
  template < typename TermT > TermT& access_term()
  {
    Handle<TermT> term( parent()->get_child( TermT::type_name() ) );
    if( is_null( term ) )
    {
      // does not exist so create the concrete term
      term = Handle<TermT>( parent()->template create_component< TermT >( TermT::type_name() ) );

      // configure the fields
      term->configure_option_recursively( Tags::solution(),   parent()->handle<CellTerm>()->solution()   );
      term->configure_option_recursively( Tags::residual(),   parent()->handle<CellTerm>()->residual()   );
      term->configure_option_recursively( Tags::wave_speed(), parent()->handle<CellTerm>()->wave_speed() );
    }

    return *term;
  }

}; // CellLoop


////////////////////////////////////////////////////////////////////////////////////////////

/// CellLoopT1 defines a functor taking the type that boost::mpl::for_each passes.
/// It is the core of the looping mechanism over Cells.
/// This CellLoopT1 is independent of the physics
template < typename ACTION >
struct CellLoopT1 : public CellLoop
{
  /// Constructor
  CellLoopT1( const std::string& name ) : CellLoop(name) {  regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "CellLoopT1<" + ACTION::type_name() + ">"; }

  /// execute the action
  virtual void execute ()
  {
    boost::mpl::for_each< typename RDM::AllCellTypes >( boost::ref(*this) );
  }

  /// operator needed for the loop over element types (SF)
  template < typename SF >
  void operator() ( SF& )
  {
    if( is_null(parent()->handle<ACTION>()) )
      throw common::SetupError(FromHere(), type_name() + " was intantiated with wrong action");

    // definition of the quadrature type
    typedef typename RDM::DefaultQuadrature<SF>::type QD;
    // parametrization of the numerical term
    typedef typename ACTION::template Term< SF, QD > TermT;

    // loop on the (sub)regions that hold elements of this type

    boost_foreach(mesh::Elements& elements,
                  common::find_components_recursively_with_filter<mesh::Elements>(*current_region,IsElementType<SF>()))
    {

      TermT& term = this->access_term<TermT>();

      // point the term to the elements of the (sub)region
      term.set_elements(elements);

      const Uint nb_elem = elements.size();
      for ( Uint elem = 0; elem != nb_elem; ++elem )
      {
        term.select_loop_idx(elem);
        term.execute();
      }
    }
  }

}; // CellLoopT1

////////////////////////////////////////////////////////////////////////////////////////////

/// CellLoopT defines a functor taking the type that boost::mpl::for_each passes.
/// It is the core of the looping mechanism over Cells.
/// This CellLoopT takes a parametrization with the physics.
template < typename ACTION, typename PHYS>
struct CellLoopT : public CellLoop
{
  /// Constructor
  CellLoopT( const std::string& name ) : CellLoop(name) {  regist_typeinfo(this); }

  /// Get the class name
  static std::string type_name () { return "CellLoopT<" + ACTION::type_name() + "," + PHYS::type_name() + ">"; }

  /// execute the action
  virtual void execute ()
  {
    boost::mpl::for_each< typename RDM::CellTypes< PHYS::MODEL::_ndim >::Cells >( boost::ref(*this) );
  }

  /// operator needed for the loop over element types (SF)
  template < typename SF >
  void operator() ( SF& )
  {
    if( is_null(parent()->handle<ACTION>()) )
      throw common::SetupError(FromHere(), type_name() + " was intantiated with wrong action");

    // definition of the quadrature type
    typedef typename RDM::DefaultQuadrature<SF>::type QD;
    // parametrization of the numerical term
    typedef typename ACTION::template Term< SF, QD, PHYS > TermT;

    // loop on the (sub)regions that hold elements of this type

    boost_foreach(mesh::Elements& elements,
                  common::find_components_recursively_with_filter<mesh::Elements>(*current_region,IsElementType<SF>()))
    {

      TermT& term = this->access_term<TermT>();

      // point the term to the elements of the (sub)region
      term.set_elements(elements);

      const Uint nb_elem = elements.size();
      for ( Uint elem = 0; elem != nb_elem; ++elem )
      {
        term.select_loop_idx(elem);
        term.execute();
      }
    }
  }

}; // CellLoopT

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_CellLoop_hpp
