// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "sdm/BCFunction.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

// Commented out unnecessary builders for now

common::ComponentBuilder<BCFunction<1u,1u>,BC,LibSDM> bcfunction_1eq_1d_builder;
common::ComponentBuilder<BCFunction<1u,2u>,BC,LibSDM> bcfunction_1eq_2d_builder;
common::ComponentBuilder<BCFunction<1u,3u>,BC,LibSDM> bcfunction_1eq_3d_builder;

//common::ComponentBuilder<BCFunction<2u,1u>,BC,LibSDM> bcfunction_2eq_1d_builder;
//common::ComponentBuilder<BCFunction<2u,2u>,BC,LibSDM> bcfunction_2eq_2d_builder;
//common::ComponentBuilder<BCFunction<2u,3u>,BC,LibSDM> bcfunction_2eq_3d_builder;

common::ComponentBuilder<BCFunction<3u,1u>,BC,LibSDM> bcfunction_3eq_1d_builder;
//common::ComponentBuilder<BCFunction<3u,2u>,BC,LibSDM> bcfunction_3eq_2d_builder;
//common::ComponentBuilder<BCFunction<3u,3u>,BC,LibSDM> bcfunction_3eq_3d_builder;

//common::ComponentBuilder<BCFunction<4u,1u>,BC,LibSDM> bcfunction_4eq_1d_builder;
common::ComponentBuilder<BCFunction<4u,2u>,BC,LibSDM> bcfunction_4eq_2d_builder;
//common::ComponentBuilder<BCFunction<4u,3u>,BC,LibSDM> bcfunction_4eq_3d_builder;

//common::ComponentBuilder<BCFunction<5u,1u>,BC,LibSDM> bcfunction_5eq_1d_builder;
//common::ComponentBuilder<BCFunction<5u,2u>,BC,LibSDM> bcfunction_5eq_2d_builder;
common::ComponentBuilder<BCFunction<5u,3u>,BC,LibSDM> bcfunction_5eq_3d_builder;

/////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
