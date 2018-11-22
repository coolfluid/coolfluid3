// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.



#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <string>
#include "common/EigenAssertions.hpp"
#include <Eigen/Dense>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include <common/EventHandler.hpp>

#include <vector>
#include <iostream>
// #include "math/LSS/System.hpp"
#include <cmath>


#include "mesh/Region.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "DirectDifferentiationSensitivityDisk.hpp"
#include "../Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/SurfaceIntegration.hpp"

namespace cf3
{

namespace UFEM
{

namespace adjoint
{

using namespace solver::actions::Proto;
using namespace std;
using namespace Eigen;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < DirectDifferentiationSensitivityDisk, common::Action, LibUFEMAdjoint > DirectDifferentiationSensitivityDisk_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

DirectDifferentiationSensitivityDisk::DirectDifferentiationSensitivityDisk(const std::string& name) :
  Action(name)
{
    properties().add("sensitivity", 0.0);
    options().add("th", m_th)
        .pretty_name("Mesh finesse")
        .description("Mesh finesse")
        .link_to(&m_th)
        .mark_basic();

    options().add("ct", m_ct)
        .pretty_name("thrust coefficient")
        .description("thrust coefficient")
        .attach_trigger(boost::bind(&DirectDifferentiationSensitivityDisk::trigger_ct, this))
        .mark_basic();
    
    options().add("area", m_area)
        .pretty_name("Area")
        .description("Area of the disk")
        .link_to(&m_area)
        .mark_basic();

    options().add("nDisk", m_nDisk)
        .pretty_name("NumberDisk")
        .description("Number of the disk")
        .link_to(&m_nDisk)
        .mark_basic();
}

DirectDifferentiationSensitivityDisk::~DirectDifferentiationSensitivityDisk()
{
}

void DirectDifferentiationSensitivityDisk::execute()
{
    // if (m_loop_regions.size() != 1)
    //     throw common::SetupError(FromHere(), "Need exactly one region for DirectDiffrentiationSensitivityDisk");

    std::vector<Handle<mesh::Region>> disk_region(1, m_loop_regions[m_nDisk]);

    FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
    FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
    FieldVariable<2, VectorField> SensU("SensU", "sensitivity_solution");
    FieldVariable<3, ScalarField> SensP("SensP", "sensitivity_solution");
  
    Real integral1 = 0.0;
    Real integral2 = 0.0;

    const auto pow2 = make_lambda([](Real x ){
	  return x*x;
	});
    const auto pow3 = make_lambda([](Real x ){
	  return x*x*x;
    });
    Real uDisk = 0.0;
    surface_integral(uDisk, disk_region, u * normal);
    uDisk /= m_area;
    Real duDisk = 0.0;
    surface_integral(duDisk, disk_region, SensU * normal);
    duDisk /= m_area;

    CFinfo << "uDisk = " << uDisk << CFendl;
    CFinfo << "duDisk = " << duDisk << CFendl;
    //auto part1 = -2 * pow3(uDisk)/m_th * m_area/(pow2(1-m_a));
    //auto part2 = -6 * pow2(uDisk)/m_th * m_area * m_a/(1 - m_a) * duDisk;
    Real part1 = 0.0;
    surface_integral(part1, disk_region, 1 * normal[0]);
    CFinfo << "Disk area = " << part1 << CFendl;
    Real part2 = 0.0;
    surface_integral(part1, disk_region, -2 * pow3(uDisk)/m_th /(pow2(1-m_a)) * normal[0]);
    surface_integral(part2, disk_region, -6 * pow2(uDisk)/m_th * m_a/(1 - m_a) * duDisk * normal[0]);
    CFinfo << "m_th = " << m_th << CFendl;
    CFinfo << "m_a = " << m_a << CFendl;
    CFinfo << "part 1 " << part1 << CFendl;
    CFinfo << "part 2 " << part2 << CFendl;
    // surface_integral(integral1, disk_region, -2*_abs(pow3((u*normal/_norm(normal))[0]))*_norm(normal) / (m_th * std::pow(m_a - 1, 2)));
    // surface_integral(integral2, disk_region, -6*pow2((u*normal/_norm(normal))[0])*_norm(normal) * m_a/(m_th*(m_a - 1)) * (SensU * normal)[0]);
    // properties().set("sensitivity", integral1 + integral2);
    properties().set("sensitivity", part1 + part2);
}



void DirectDifferentiationSensitivityDisk::trigger_ct()
{
  // Copy ct values
  m_ct = options().value<Real>("ct");

  // Update a values
  m_a = (1.-std::sqrt(1-m_ct))/2.0;

}



} // namespace adjointtube

} // namespace UFEM

} // namespace cf3
