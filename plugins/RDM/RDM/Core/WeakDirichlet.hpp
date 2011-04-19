// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_WeakDirichlet_hpp
#define CF_RDM_WeakDirichlet_hpp

#include <iostream> // to remove

#include "Math/VectorialFunction.hpp"

#include "RDM/Core/BoundaryTerm.hpp"
#include "RDM/Core/BcBase.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Mesh { class CMesh; class CField; }

namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API WeakDirichlet : public RDM::BoundaryTerm {
public: // typedefs

  /// the actual BC implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  /// pointers
  typedef boost::shared_ptr<WeakDirichlet> Ptr;
  typedef boost::shared_ptr<WeakDirichlet const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  WeakDirichlet ( const std::string& name );

  /// Virtual destructor
  virtual ~WeakDirichlet() {};

  /// Get the class name
  static std::string type_name () { return "WeakDirichlet"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_mesh();
  void config_function();

private: // data

  /// access to the solution field on the mesh
  boost::weak_ptr<Mesh::CField> m_solution;
  /// function parser for the math formula of the dirichlet condition
  Math::VectorialFunction  m_function;

}; // !WeakDirichlet

//------------------------------------------------------------------------------------------

template < typename SF, typename QD, typename PHYS >
class RDM_API WeakDirichlet::Term : public BcBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef BcBase<SF,QD,PHYS> B;
  /// pointers
  typedef boost::shared_ptr< Term > Ptr;
  typedef boost::shared_ptr< Term const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
 Term ( const std::string& name ) : BcBase<SF,QD,PHYS>(name) {}

 /// Get the class name
 static std::string type_name () { return "WeakDirichlet.BC<" + SF::type_name() + ">"; }

 /// execute the action
 virtual void execute ()
 {
//   std::cout << "Face [" << B::idx() << "]" << std::endl;


 }

}; // !WeakDirichlet::Term

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_WeakDirichlet_hpp
