#include "Common/MyInt.hh"

using namespace COOLFluiD::Common;

MyInt::MyInt(int value)
 : m_value(value)
{
 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int MyInt::getValue() const
{
 return m_value;
}