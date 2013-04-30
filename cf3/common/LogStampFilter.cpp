// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/LogStampFilter.hpp"

using namespace cf3::common;

LogStampFilter::LogStampFilter(const std::string & streamName,
                               const std::string & stamp)
: m_place("", 0, ""),
m_stamp(stamp),
m_streamName(streamName),
m_newMessage(true)
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStampFilter::setStamp(const std::string & stamp)
{
  m_stamp = stamp;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

std::string LogStampFilter::getStamp() const
{
  return m_stamp;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStampFilter::endMessage()
{
  m_newMessage = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStampFilter::setPlace(const CodeLocation & place)
{
  m_place = place;
}
