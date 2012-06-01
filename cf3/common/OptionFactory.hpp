// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OptionFactory_hpp
#define cf3_common_OptionFactory_hpp

///////////////////////////////////////////////////////////////////////////////

#include "common/Option.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////

/// Interface for option builders that can be registered with the factory
class Common_API OptionBuilder
{
public:
  virtual ~OptionBuilder() {}

  /// Create an option with the given default value, passed as a string or vector of strings
  virtual boost::shared_ptr<Option> create_option(const std::string& name, const boost::any& default_value) = 0;
};

/////////////////////////////////////////////////////////////////////////////

/// Factory class to buikd options dynamically
class Common_API OptionFactory
{

public:

  /// Singleton instance
  static OptionFactory& instance();

  /// Register a builder with the given type
  void register_builder(const std::string& type, const boost::shared_ptr<OptionBuilder>& builder);

  /// Create an option with the given type and default value, passed as a string or vector of strings
  boost::shared_ptr<Option> create_option(const std::string& name, const std::string& type, const boost::any& default_value);

private:
  OptionFactory();
  std::map< std::string, boost::shared_ptr<OptionBuilder> > m_builders;
};

/////////////////////////////////////////////////////////////////////////////

/// Helper class to register builders
struct Common_API RegisterOptionBuilder
{
  /// Constructor registers the builder in the factory. Takes ownership of the passed pointer.
  RegisterOptionBuilder(const std::string& type, OptionBuilder* builder);
};

/////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

///////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionFactory_hpp
