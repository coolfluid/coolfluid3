#include <string>

#include "CodeLocation.hh"

#include "CFLogStampFilter.hh"

using namespace CF::Common;

CFLogStampFilter::CFLogStampFilter(const std::string & streamName, 
                                   const std::string & stamp)
 : m_stamp(stamp),
   m_newMessage(true),
   m_streamName(streamName),
   m_place("", 0, "")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStampFilter::setStamp(const std::string & stamp)
{
 m_stamp = stamp;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

std::string CFLogStampFilter::getStamp() const
{
 return m_stamp;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStampFilter::endMessage()
{
 m_newMessage = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStampFilter::setPlace(const CodeLocation & place)
{
 m_place = place;
}
