#include "Common/xmlParser.h"

#include "Common/BuilderParserFrameInfo.hh"

#include <iostream>

using namespace std;
using namespace CF;
using namespace CF::Common;

BuilderParserFrameInfo::BuilderParserFrameInfo()
{
 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void BuilderParserFrameInfo::setFrameType(unsigned int frameType)
{
 this->clear();
 this->frameType = frameType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void BuilderParserFrameInfo::clear()
{
 this->frameAttributes.clear();
 this->frameData.deleteNodeContent();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

string BuilderParserFrameInfo::getAttribute(const string & attrName, bool * ok) const
{
 map<string, string>::const_iterator it = this->frameAttributes.find(attrName);
 bool found = it != this->frameAttributes.end() && it->first == attrName;
 
 if(ok != CFNULL)
  *ok = found;
 
 if(found)
  return it->second;
 
 return "";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool BuilderParserFrameInfo::isAttributeSet(const string & attrName, 
                                            bool emptyAllowed) const
{
 bool ok;
 string value = this->getAttribute(attrName, &ok);
 
 if(!ok)
  return false;
 
 return emptyAllowed || !value.empty();
}
