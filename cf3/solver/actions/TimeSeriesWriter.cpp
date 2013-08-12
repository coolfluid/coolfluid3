// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"

#include "solver/actions/TimeSeriesWriter.hpp"
#include "solver/Tags.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TimeSeriesWriter, common::Action, LibActions > TimeSeriesWriter_Builder;

///////////////////////////////////////////////////////////////////////////////////////

TimeSeriesWriter::TimeSeriesWriter ( const std::string& name ) :
  common::Action(name)
{  
  options().add(Tags::time(), m_time)
    .pretty_name("Time")
    .description("Time component governing the reference time for the time series write")
    .mark_basic()
    .link_to(&m_time);

  options().add("interval", 1u)
    .pretty_name("Interval")
    .description("Write every interval timesteps")
    .mark_basic()
    .link_to(&m_interval);
}

/////////////////////////////////////////////////////////////////////////////////////

void TimeSeriesWriter::execute()
{
  if(is_null(m_time))
    throw common::SetupError(FromHere(), "Time component is not configured for " + uri().path());

  Uint current_iter = m_time->iter();
  if(current_iter % m_interval != 0)
    return;

  const std::string current_time_str = common::to_str(m_time->current_time());
  const std::string current_iter_str = common::to_str(current_iter);


  BOOST_FOREACH(common::Action& action, common::find_components<common::Action>(*this))
  {
    if(action.options().check("file"))
    {
      const common::URI original_uri = action.options().value<common::URI>("file");
      std::string rewritten_path = original_uri.path();
      boost::algorithm::replace_all(rewritten_path, "{time}", current_time_str);
      boost::algorithm::replace_all(rewritten_path, "{iteration}", current_iter_str);
      action.options().set("file", common::URI(rewritten_path, original_uri.scheme()));
      action.execute();
      action.options().set("file", original_uri); // Set back the original URI, so we can replace the patterns on the next write
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

