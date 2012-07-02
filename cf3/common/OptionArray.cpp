// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionArray.hpp"
#include "common/OptionFactory.hpp"
#include "common/UUCount.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

template<typename TYPE>
class OptionArrayBuilder : public OptionBuilder
{
public:
  virtual boost::shared_ptr< Option > create_option(const std::string& name, const boost::any& default_value)
  {
    const std::vector<std::string> string_vec = boost::any_cast< std::vector<std::string> >(default_value);
    typename OptionArray<TYPE>::value_type def_val; def_val.reserve(string_vec.size());
    BOOST_FOREACH(const std::string& str_val, string_vec)
    {
      def_val.push_back(from_str<TYPE>(str_val));
    }
    return boost::shared_ptr<Option>(new OptionArray< TYPE >(name, def_val));
  }
};

RegisterOptionBuilder bool_array_builder(common::class_name< std::vector<bool> >(), new OptionArrayBuilder<bool>());
RegisterOptionBuilder int_array_builder(common::class_name< std::vector<int> >(), new OptionArrayBuilder<int>());
RegisterOptionBuilder string_array_builder(common::class_name< std::vector<std::string> >(), new OptionArrayBuilder<std::string>());
RegisterOptionBuilder uint_array_builder(common::class_name< std::vector<Uint> >(), new OptionArrayBuilder<Uint>());
RegisterOptionBuilder Real_array_builder(common::class_name< std::vector<Real> >(), new OptionArrayBuilder<Real>());
RegisterOptionBuilder uri_array_builder(common::class_name< std::vector<URI> >(), new OptionArrayBuilder<URI>());

} // common
} // cf3

