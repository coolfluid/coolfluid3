#include <string>

#include "CodeLocation.hh"

#include "LogStampFilter.hh"

using namespace CF::Common;

LogStampFilter::LogStampFilter(const std::string & streamName, 
                                   const std::string & stamp)
 : m_stamp(stamp),
   m_newMessage(true),
   m_streamName(streamName),
   m_place("", 0, "")
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
