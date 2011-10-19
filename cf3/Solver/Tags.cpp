// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Solver/Tags.hpp"

const char* cf3::Solver::Tags::domain()
{
  return "domain";
}

const char* cf3::Solver::Tags::solver()
{
  return "solver";
}

const char* cf3::Solver::Tags::mesh()
{
  return "mesh";
}

const char* cf3::Solver::Tags::regions()
{
  return "regions";
}

const char* cf3::Solver::Tags::time()
{
  return "ctime";
}

const char* cf3::Solver::Tags::physical_model()
{
  return "physical_model";
}
