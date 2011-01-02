// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_CDummyLoopOperation_hpp
#define CF_Actions_CDummyLoopOperation_hpp

#include "Mesh/CFieldElements.hpp"

#include "Actions/CNodeOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
	
  namespace Mesh
  {
		class CFieldElements;
    template <typename T> class CList;
	}

///////////////////////////////////////////////////////////////////////////////////////

class CDummyLoopOperation : public Actions::CNodeOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CDummyLoopOperation> Ptr;
  typedef boost::shared_ptr<CDummyLoopOperation const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CDummyLoopOperation ( const std::string& name );

  /// Virtual destructor
  virtual ~CDummyLoopOperation() {};

  /// Get the class name
  static std::string type_name () { return "CDummyLoopOperation"; }

  /// Set the loop_helper
  void create_loop_helper (CF::Mesh::CElements& geometry_elements );
	
  /// execute the action
  virtual void execute ();
	
	/// @return the nodes to loop over
  virtual Mesh::CList<Uint>& loop_list () const;
		
private: // data
	
  struct LoopHelper
  {
    LoopHelper(Mesh::CElements& geometry_elements) :
      used_nodes(geometry_elements.used_nodes())
    { }
    Mesh::CList<Uint>& used_nodes;
  };
	
  boost::shared_ptr<LoopHelper> m_loop_helper;

};

} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Actions_CDummyLoopOperation_hpp
