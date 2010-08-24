#include "Common/BasicExceptions.hpp"
#include "Common/ObjectProvider.hpp"
#include "Common/CommonLib.hpp"
#include "Common/XmlHelpers.hpp"

#include "Common/CLink.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CLink, Component, CommonLib, NB_ARGS_1 >
CLink_Provider ( CLink::type_name() );

////////////////////////////////////////////////////////////////////////////////

CLink::CLink ( const CName& name) : Component ( name )
{
  BUILD_COMPONENT;
  m_is_link = true;

  regist_signal("change_link", "Change link path")->connect(boost::bind(&CLink::change_link, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

CLink::~CLink()
{
}

////////////////////////////////////////////////////////////////////////////////

Component::Ptr CLink::get ()
{
  return m_link_component.lock();
}

Component::ConstPtr CLink::get () const
{
  return m_link_component.lock();
}

////////////////////////////////////////////////////////////////////////////////

void CLink::link_to ( Component::Ptr lnkto )
{
  if (lnkto->is_link())
    throw SetupError(FromHere(), "Cannot link a CLink to another CLink");

  m_link_component = lnkto;
}

////////////////////////////////////////////////////////////////////////////////

void CLink::change_link( XmlNode & node )
{
  XmlParams p(node);

  std::string path = p.get_param<std::string>("target_path");
  Component::Ptr target = m_root.lock()->look_component(path);

  link_to (target);

  XmlNode * reply = XmlOps::add_reply_frame(node);

  XmlParams(*reply).add_param("target_path", path);
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
