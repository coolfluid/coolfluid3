#include <QtCore>

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/CBrowser.hpp"

using namespace CF::GUI::Client;

CBrowser::CBrowser()
  : Component(CLIENT_BROWSERS),
    m_counter(0)
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CBrowser::generateName()
{
 return QString("Browser_%1").arg(m_counter++);
}
