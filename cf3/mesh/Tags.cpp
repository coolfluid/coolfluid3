// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

const char * Tags::normal () { return "normal"; }
const char * Tags::area ()   { return "area"; }
const char * Tags::volume () { return "volume"; }

const char * Tags::geometry ()     { return "geometry"; }
const char * Tags::topology ()     { return "topology"; }
const char * Tags::coordinates ()  { return "coordinates"; }
const char * Tags::nodes ()        { return "nodes"; }
const char * Tags::nodes_used ()   { return "nodes_used"; }

const char * Tags::global_indices ()  { return "global_indices"; }
const char * Tags::map_global_to_local ()  { return "map_global_to_local"; }

const char * Tags::cell_entity ()  { return "cell_entity"; }
const char * Tags::face_entity ()  { return "face_entity"; }
const char * Tags::edge_entity ()  { return "edge_entity"; }
const char * Tags::point_entity()  { return "point_entity"; }

const char * Tags::interface ()    { return "interface"; }

const char * Tags::cells ()  { return "cells"; }
const char * Tags::inner_faces ()  { return "inner_faces"; }
const char * Tags::outer_faces ()  { return "outer_faces"; }
const char * Tags::bdry_faces ()  { return "bdry_faces"; }

const char * Tags::connectivity_table () { return "connectivity_table"; }

const char * Tags::event_mesh_loaded() { return "mesh_loaded"; }
const char * Tags::event_mesh_changed() { return "mesh_changed"; }

//const char * Tags::geometry_elements () { return "geometry_elements"; }

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
