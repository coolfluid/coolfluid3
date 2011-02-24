// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CBubbleRemove_hpp
#define CF_Mesh_CBubbleRemove_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
//////////////////////////////////////////////////////////////////////////////

/// Mesh transformer that enriches the lagrangian space with bubble functions
/// in each element
/// @author Tiago Quintino
class Mesh_Actions_API CBubbleRemove : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CBubbleRemove> Ptr;
    typedef boost::shared_ptr<CBubbleRemove const> ConstPtr;

private: // typedefs
  
public: // functions
  
  /// constructor
  CBubbleRemove( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CBubbleRemove"; }

  virtual void execute();
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;
  
private: // functions
  
}; // CBubbleRemove


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CBubbleRemove_hpp
