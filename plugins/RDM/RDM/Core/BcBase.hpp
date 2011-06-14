// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_BcBase_hpp
#define CF_RDM_BcBase_hpp

#include <functional>

#include "Common/Core.hpp"
#include "Common/OptionT.hpp"
#include "Common/BasicExceptions.hpp"

#include "Mesh/ElementData.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"

#include "Solver/Actions/CLoopOperation.hpp"

#include "RDM/Core/LibCore.hpp"
#include "RDM/Core/FaceLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SF, typename QD, typename PHYS >
class RDM_CORE_API BcBase : public Solver::Actions::CLoopOperation {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr< BcBase > Ptr;
  typedef boost::shared_ptr< BcBase const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  BcBase ( const std::string& name );

  /// Virtual destructor
  virtual ~BcBase() {};

  /// Get the class name
  static std::string type_name () { return "BcBase<" + SF::type_name() + ">"; }
	
protected: // helper functions

  void change_elements()
  { 
    /// @todo improve this (ugly)

    connectivity_table = elements().as_ptr<Mesh::CElements>()->node_connectivity().as_ptr< Mesh::CTable<Uint> >();
    coordinates = elements().nodes().coordinates().as_ptr< Mesh::CTable<Real> >();

    cf_assert( is_not_null(connectivity_table) );

    /// @todo modify these to option components configured from

    Mesh::CField::Ptr csolution = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( Common::Core::instance().root(), "solution" );
    cf_assert( is_not_null( csolution ) );
    solution = csolution->data_ptr();

    Mesh::CField::Ptr cresidual = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( Common::Core::instance().root(), "residual" );
    cf_assert( is_not_null( cresidual ) );
    residual = cresidual->data_ptr();

    Mesh::CField::Ptr cwave_speed = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( Common::Core::instance().root(), "wave_speed" );
    cf_assert( is_not_null( cwave_speed ) );
    wave_speed = cwave_speed->data_ptr();
  }

protected: // typedefs

  typedef typename SF::NodeMatrixT                             NodeMT;

  typedef Eigen::Matrix<Real, QD::nb_points, 1u>               WeightVT;

  typedef Eigen::Matrix<Real, QD::nb_points, PHYS::neqs>       ResidualMT;

  typedef Eigen::Matrix<Real, PHYS::neqs, PHYS::neqs >         EigenValueMT;

  typedef Eigen::Matrix<Real, PHYS::neqs, PHYS::neqs>          PhysicsMT;
  typedef Eigen::Matrix<Real, PHYS::neqs, 1u>                  PhysicsVT;

  typedef Eigen::Matrix<Real, SF::nb_nodes,   PHYS::neqs>      SolutionMT;
  typedef Eigen::Matrix<Real, 1u, PHYS::neqs >                 SolutionVT;

  typedef Eigen::Matrix<Real, QD::nb_points, SF::nb_nodes>     SFMatrixT;
  typedef Eigen::Matrix<Real, 1u, SF::nb_nodes >               SFVectorT;

  typedef Eigen::Matrix<Real, PHYS::ndim, 1u>                  DimVT;

  typedef Eigen::Matrix<Real, QD::nb_points, PHYS::ndim>       QCoordMT;
  typedef Eigen::Matrix<Real, QD::nb_points, PHYS::neqs>       QSolutionMT;
  typedef Eigen::Matrix<Real, PHYS::neqs, PHYS::ndim>          QSolutionVT;

protected: // data

  /// pointer to connectivity table, may reset when iterating over element types
  Mesh::CTable<Uint>::Ptr connectivity_table;
  /// pointer to nodes coordinates, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr coordinates;
  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr solution;
  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr residual;
  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr wave_speed;

  /// physical properties
  typename PHYS::Properties phys_props;

};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SF, typename QD, typename PHYS>
BcBase<SF,QD,PHYS>::BcBase ( const std::string& name ) :
  Solver::Actions::CLoopOperation(name)
{ 
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &BcBase<SF,QD,PHYS>::change_elements, this ) );
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_BcBase_hpp
