#include "Common/CodeLocation.hh"
#include "Common/LogStampFilter.hh"

using namespace CF::Common;

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
