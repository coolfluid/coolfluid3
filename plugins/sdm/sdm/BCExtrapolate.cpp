// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "sdm/BCExtrapolate.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<BCExtrapolate<1u,1u>,BC,LibSDM> bcextrapolate_1eq_1d_builder;
common::ComponentBuilder<BCExtrapolate<1u,2u>,BC,LibSDM> bcextrapolate_1eq_2d_builder;
common::ComponentBuilder<BCExtrapolate<1u,3u>,BC,LibSDM> bcextrapolate_1eq_3d_builder;

common::ComponentBuilder<BCExtrapolate<3u,1u>,BC,LibSDM> bcextrapolate_3eq_1d_builder;
common::ComponentBuilder<BCExtrapolate<4u,2u>,BC,LibSDM> bcextrapolate_4eq_2d_builder;
common::ComponentBuilder<BCExtrapolate<5u,3u>,BC,LibSDM> bcextrapolate_5eq_3d_builder;

/////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
