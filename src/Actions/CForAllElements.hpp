// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CForAllElements_hpp
#define CF_Mesh_CForAllElements_hpp

#include "Mesh/CRegion.hpp"
#include "Mesh/CArray.hpp"

#include "Actions/CAction.hpp"
#include "Actions/CElementOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;
using namespace CF::Common::String;
namespace CF {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

class Actions_API CForAllElements : public CAction
{
public: // typedefs

  typedef boost::shared_ptr< CForAllElements > Ptr;
  typedef boost::shared_ptr< CForAllElements const > ConstPtr;
  friend struct Looper;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllElements ( const CName& name ) :
    CAction(name)
  {
    BUILD_COMPONENT;
    m_property_list["Regions"].as_option().attach_trigger ( boost::bind ( &CForAllElements::trigger_Regions,   this ) );
  }

  void trigger_Regions()
  {
    std::vector<URI> vec; property("Regions").put_value(vec);
    BOOST_FOREACH(const CPath region_path, vec)
    {
      m_loop_regions.push_back(look_component_type<CRegion>(region_path));
    }
  }

  /// Virtual destructor
  virtual ~CForAllElements() {}

  /// Get the class name
  static std::string type_name () { return "CForAllElements"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options )
  {
    std::vector< URI > dummy;
    options.add_option< OptionArrayT < URI > > ("Regions", "Regions to loop over", dummy)->mark_basic();
  }

  // functions specific to the CForAllElements component

  CElementOperation& create_action(const std::string action_provider)
  {
    // The execuation of operations must be in chronological order, hence
    // they get an alphabetical name
    std::string name = action_provider;
    CElementOperation::Ptr sub_operation = 
      (create_component_abstract_type<CElementOperation>(action_provider,name));
    add_component(sub_operation);
    return *sub_operation;
  }

  const CElementOperation& action(const CName& name) const
  {
    return *get_child_type<CElementOperation const>(name);
  }

  CElementOperation& action(const CName& name)
  {
    return *get_child_type<CElementOperation>(name);
  }

  void execute()
  {
    BOOST_FOREACH(CRegion::Ptr& region, m_loop_regions)
      BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(*region))
    {
      // Setup all child operations
      BOOST_FOREACH(CElementOperation& op, range_typed<CElementOperation>(*this))
      {
        op.set_loophelper( elements );
        const Uint elem_count = elements.elements_count();
        for ( Uint elem = 0; elem != elem_count; ++elem )
        {
          op.set_element_idx(elem);
          op.execute();
        }
      }
    }
  }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private:

  /// Regions to loop over
  std::vector<CRegion::Ptr> m_loop_regions;
  
  Uint m_counter;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CForAllElements_hpp
