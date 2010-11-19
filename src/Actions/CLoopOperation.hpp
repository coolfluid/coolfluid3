// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_CLoopOperation_hpp
#define CF_Actions_CLoopOperation_hpp

#include "Actions/CAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
  
namespace Mesh {
  class CElements;
	template <typename T> class CList;
}

namespace Actions {

  using namespace CF::Mesh;

///////////////////////////////////////////////////////////////////////////////////////

class Actions_API CLoopOperation : public CAction
{
public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < CLoopOperation , Common::NB_ARGS_1 > PROVIDER;

  /// pointers
  typedef boost::shared_ptr<CLoopOperation> Ptr;
  typedef boost::shared_ptr<CLoopOperation const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CLoopOperation ( const CName& name );

  /// Virtual destructor
  virtual ~CLoopOperation() {};

  /// Get the class name
  static std::string type_name () { return "CLoopOperation"; }

  /// Configuration Options
  static void define_config_properties ( Common::PropertyList& options );
  
  virtual void set_loophelper ( CElements& geometry_elements ) = 0;
  
  void set_loop_idx ( const Uint idx ) { m_idx = idx; }
	
	virtual CList<Uint>& loop_list();
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

protected: // data

  Uint m_idx;
};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Actions_CLoopOperation_hpp
