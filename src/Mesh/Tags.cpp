// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

const char * Tags::normal () { return "normal"; }
const char * Tags::area ()   { return "area"; }
const char * Tags::volume () { return "volume"; }

const char * Tags::coordinates ()  { return "coordinates"; }
const char * Tags::nodes ()        { return "nodes"; }
const char * Tags::nodes_used ()   { return "nodes_used"; }

const char * Tags::global_elem_indices ()  { return "gelemidx"; }
const char * Tags::global_node_indices ()  { return "gnodeidx"; }

const char * Tags::cell_entity ()  { return "cell_entity"; }
const char * Tags::face_entity ()  { return "face_entity"; }
const char * Tags::edge_entity ()  { return "edge_entity"; }
const char * Tags::point_entity()  { return "point_entity"; }

const char * Tags::interface ()    { return "interface"; }

const char * Tags::inner_faces ()  { return "inner_faces"; }
const char * Tags::outer_faces ()  { return "outer_faces"; }

const char * Tags::connectivity_table () { return "connectivity_table"; }

const char * Tags::geometry_elements () { return "geometry_elements"; }

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
