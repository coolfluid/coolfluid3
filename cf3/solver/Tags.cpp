// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "solver/Tags.hpp"

const char* cf3::solver::Tags::domain()
{
  return "domain";
}

const char* cf3::solver::Tags::solver()
{
  return "solver";
}

const char* cf3::solver::Tags::mesh()
{
  return "mesh";
}

const char* cf3::solver::Tags::regions()
{
  return "regions";
}

const char* cf3::solver::Tags::time()
{
  return "time";
}

const char* cf3::solver::Tags::physical_model()
{
  return "physical_model";
}
