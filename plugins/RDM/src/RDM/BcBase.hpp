// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_BcBase_hpp
#define cf3_RDM_BcBase_hpp

#include <functional>

#include "common/Core.hpp"
#include "common/OptionT.hpp"
#include "common/BasicExceptions.hpp"

#include "mesh/ElementData.hpp"
#include "mesh/Field.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/ElementType.hpp"

#include "solver/Actions/CLoopOperation.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/FaceLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SF, typename QD, typename PHYS >
class RDM_API BcBase : public solver::Actions::CLoopOperation {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr< BcBase > Ptr;
  typedef boost::shared_ptr< BcBase const> ConstPtr;

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  /// Contructor
  /// @param name of the component
  BcBase ( const std::string& name );

  /// Virtual destructor
  virtual ~BcBase() {}

  /// Get the class name
  static std::string type_name () { return "BcBase<" + SF::type_name() + ">"; }

protected: // helper functions

  void change_elements()
  {
    connectivity =
        elements().as_ptr<mesh::Elements>()->node_connectivity().as_ptr< mesh::Connectivity >();
    coordinates =
        elements().geometry().coordinates().as_ptr< mesh::Field >();

    cf3_assert( is_not_null(connectivity) );
    cf3_assert( is_not_null(coordinates) );

    solution   = csolution.lock();
    residual   = cresidual.lock();
    wave_speed = cwave_speed.lock();
  }

protected: // typedefs

  typedef typename SF::NodesT                                     NodeMT;

  typedef Eigen::Matrix<Real, QD::nb_points, 1u>                       WeightVT;

  typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_neqs>       ResidualMT;

  typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs, PHYS::MODEL::_neqs > EigenValueMT;

  typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs, PHYS::MODEL::_neqs>  PhysicsMT;
  typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs, 1u>                  PhysicsVT;

  typedef Eigen::Matrix<Real, SF::nb_nodes,   PHYS::MODEL::_neqs>      SolutionMT;
  typedef Eigen::Matrix<Real, 1u, PHYS::MODEL::_neqs >                 SolutionVT;

  typedef Eigen::Matrix<Real, QD::nb_points, SF::nb_nodes>             SFMatrixT;
  typedef Eigen::Matrix<Real, 1u, SF::nb_nodes >                       SFVectorT;

  typedef Eigen::Matrix<Real, PHYS::MODEL::_ndim, 1u>                  DimVT;

  typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_ndim>       QCoordMT;
  typedef Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_neqs>       QSolutionMT;
  typedef Eigen::Matrix<Real, PHYS::MODEL::_neqs, PHYS::MODEL::_ndim>  QSolutionVT;

protected: // data

  boost::weak_ptr< mesh::Field > csolution;   ///< solution field
  boost::weak_ptr< mesh::Field > cresidual;   ///< residual field
  boost::weak_ptr< mesh::Field > cwave_speed; ///< wave_speed field

  /// pointer to connectivity table, may reset when iterating over element types
  mesh::Connectivity::Ptr connectivity;
  /// pointer to nodes coordinates, may reset when iterating over element types
  mesh::Table<Real>::Ptr coordinates;
  /// pointer to solution table, may reset when iterating over element types
  mesh::Table<Real>::Ptr solution;
  /// pointer to solution table, may reset when iterating over element types
  mesh::Table<Real>::Ptr residual;
  /// pointer to solution table, may reset when iterating over element types
  mesh::Table<Real>::Ptr wave_speed;

  typename PHYS::MODEL::Properties phys_props; ///< physical properties

};

////////////////////////////////////////////////////////////////////////////////////////////

template<typename SF, typename QD, typename PHYS>
BcBase<SF,QD,PHYS>::BcBase ( const std::string& name ) :
  solver::Actions::CLoopOperation(name)
{
  regist_typeinfo(this);

  m_options["elements"].attach_trigger ( boost::bind ( &BcBase<SF,QD,PHYS>::change_elements, this ) );
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_BcBase_hpp
