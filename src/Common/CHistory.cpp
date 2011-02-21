// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/Log.hpp"

#include "Common/XmlHelpers.hpp"

#include "Common/CHistory.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

ComponentBuilder < CHistory, Component, LibCommon > CHistory_Builder;

CHistory::CHistory(const std::string& name) :
    Component(name),m_num_it(10000)
{
  regist_signal("convergence_history", "Lists convergence history", "Get history")->
      connect( boost::bind( &CHistory::convergence_history, this, _1));

}

CHistory::~CHistory()
{

}

void CHistory::convergence_history( XmlNode & node )
{
  XmlNode & reply = *XmlOps::add_reply_frame( node );
  XmlParams p(reply);

//  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
//  CFinfo << "avant sine @ " << boost::posix_time::to_simple_string(now) << CFendl;

  sine(m_num_it);

//  now = boost::posix_time::second_clock::local_time();
//  CFinfo << "avant add_array @ " << boost::posix_time::to_simple_string(now) << CFendl;

  p.add_array("x_axis", m_x_axis);
  p.add_array("y_axis", m_y_axis);

//  now = boost::posix_time::second_clock::local_time();
//  CFinfo << "apres add_array @ " << boost::posix_time::to_simple_string(now) << CFendl;
}

void CHistory::sine(int points)
{
  m_x_axis = std::vector<double>(points);
  m_y_axis = std::vector<double>(points);

  for (double x = 0; x < points; ++x)
  {
    m_x_axis[x] = x; // x entre 0 et nbPoints
    m_y_axis[x] = (std::sin(x/(points/10)) * 20); //y entre 20 et -20
  }
  m_num_it += m_num_it;
}

} // Common
} // CF
