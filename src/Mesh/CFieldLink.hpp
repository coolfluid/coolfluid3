// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CFieldLink_hpp
#define CF_Common_CFieldLink_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLink.hpp"

namespace CF {
namespace Mesh {
  class CField2;
////////////////////////////////////////////////////////////////////////////////

  /// Component for creating links between components
  /// @author Tiago Quintino
  class Common_API CFieldLink : public Common::CLink {

  public: //typedefs

    typedef boost::shared_ptr<CFieldLink> Ptr;
    typedef boost::shared_ptr<CFieldLink const> ConstPtr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CFieldLink ( const std::string& name );

    /// Virtual destructor
    virtual ~CFieldLink();

    /// Get the class name
    static std::string type_name () { return "CFieldLink"; }

    CField2& field () { return *m_link_component.lock(); }
    const CField2& field () const { return *m_link_component.lock(); }

    // functions specific to the CFieldLink component

    /// link to component
    bool is_linked () const;

    virtual void link_to ( Component::Ptr lnkto );

  private: // data

    /// this is a link to the component
    /// using weak_ptr means it might become invalid so we should test for expire()
    boost::weak_ptr<CField2> m_link_component;

  }; // CFieldLink

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CFieldLink_hpp
