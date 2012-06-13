// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "sdm/BCConstant.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

// Commented out unnecessary builders for now

common::ComponentBuilder<BCConstant<1u,1u>,BC,LibSDM> bcconstant_1eq_1d_builder;
common::ComponentBuilder<BCConstant<1u,2u>,BC,LibSDM> bcconstant_1eq_2d_builder;
//common::ComponentBuilder<BCConstant<1u,3u>,BC,LibSDM> bcconstant_1eq_3d_builder;

//common::ComponentBuilder<BCConstant<2u,1u>,BC,LibSDM> bcconstant_2eq_1d_builder;
//common::ComponentBuilder<BCConstant<2u,2u>,BC,LibSDM> bcconstant_2eq_2d_builder;
//common::ComponentBuilder<BCConstant<2u,3u>,BC,LibSDM> bcconstant_2eq_3d_builder;

common::ComponentBuilder<BCConstant<3u,1u>,BC,LibSDM> bcconstant_3eq_1d_builder;
//common::ComponentBuilder<BCConstant<3u,2u>,BC,LibSDM> bcconstant_3eq_2d_builder;
//common::ComponentBuilder<BCConstant<3u,3u>,BC,LibSDM> bcconstant_3eq_3d_builder;

//common::ComponentBuilder<BCConstant<4u,1u>,BC,LibSDM> bcconstant_4eq_1d_builder;
common::ComponentBuilder<BCConstant<4u,2u>,BC,LibSDM> bcconstant_4eq_2d_builder;
//common::ComponentBuilder<BCConstant<4u,3u>,BC,LibSDM> bcconstant_4eq_3d_builder;

//common::ComponentBuilder<BCConstant<5u,1u>,BC,LibSDM> bcconstant_5eq_1d_builder;
//common::ComponentBuilder<BCConstant<5u,2u>,BC,LibSDM> bcconstant_5eq_2d_builder;
common::ComponentBuilder<BCConstant<5u,3u>,BC,LibSDM> bcconstant_5eq_3d_builder;

/////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<BCConstantStrong<4u,2u>,BC,LibSDM> bcconstantstrong_4eq_2d_builder;


} // sdm
} // cf3
