// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/math/special_functions/next.hpp>

#include "common/ArrayDiff.hpp"
#include "common/Builder.hpp"
#include "common/List.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/FindComponents.hpp"

#include "common/PE/Comm.hpp"

#include "common/XML/FileOperations.hpp"
#include "common/XML/XmlNode.hpp"

#include "common/LibCommon.hpp"
#include "Table.hpp"

namespace cf3 {
namespace common {

common::ComponentBuilder < common::ArrayDiff, common::Action, LibCommon> anArrayDiff_Builder;
  
namespace detail
{
  // MPL functor that does the actual difference
  struct DoArrayDiff
  {
    DoArrayDiff(const Handle<Component const>& left, const Handle<Component const>& right, const Uint max_ulps, const Real zero_threshold, bool& found_type, bool& arrays_equal) :
      m_left(left),
      m_right(right),
      m_max_ulps(max_ulps),
      m_zero_threshold(zero_threshold),
      m_found_type(found_type),
      m_arrays_equal(arrays_equal)
    {
      m_prefix = "rank " + to_str(PE::Comm::instance().rank()) + ": ";
    }
    
    template<typename NumberT>
    void operator()(const NumberT)
    {
      if(m_left->derived_type_name() != m_right->derived_type_name())
        throw common::SetupError(FromHere(), "Array types do not match");
      
      Handle< List<NumberT> const > left_list(m_left);
      Handle< List<NumberT> const > right_list(m_right);
      if(is_not_null(left_list) && is_not_null(right_list))
      {
        m_found_type = true;
        if(left_list->size() != right_list->size())
        {
          CFdebug << m_prefix << "Array sizes don't match: left: " << left_list->size() << ", right: " << right_list->size() << CFendl;
          m_arrays_equal = false;
          return;
        }
        compare_lists(boost::is_floating_point<NumberT>(), *left_list, *right_list);
      }
      else
      {
        Handle< Table<NumberT> const > left_table(m_left);
        Handle< Table<NumberT> const > right_table(m_right);
        if(is_not_null(left_table) && is_not_null(right_table))
        {
          m_found_type = true;
          if(left_table->size() != right_table->size() || left_table->row_size() != right_table->row_size())
          {
            CFdebug << m_prefix << "Array sizes don't match: left: " << left_table->size() << "x" << left_table->row_size() << ", right: " << right_table->size() << "x" << right_table->row_size() << CFendl;
            m_arrays_equal = false;
            return;
          }
          compare_tables(boost::is_floating_point<NumberT>(), *left_table, *right_table);
        }
      }
      
    }
    
    // Floating point comparison
    template<typename NumberT>
    void compare_lists(boost::true_type, const List<NumberT>& left, const List<NumberT>& right)
    {
      m_arrays_equal = true;
      const Uint nb_rows = left.size();
      for(Uint i = 0; i != nb_rows; ++i)
      {
        const Real diff = compare_numbers(left[i], right[i]);
        if(diff > m_max_ulps)
        {
          m_arrays_equal = false;
          CFdebug << m_prefix << "List mismatch at index "  << i << ": distance [" << left[i] << ", " << right[i] << "] is " << right[i]-left[i] << "(" << diff << " ulps)" << CFendl;
        }
      }
    }

    // Integer number comparison
    template<typename NumberT>
    void compare_lists(boost::false_type, const List<NumberT>& left, const List<NumberT>& right)
    {
      m_arrays_equal = true;
      const Uint nb_rows = left.size();
      for(Uint i = 0; i != nb_rows; ++i)
      {
        if(left[i] != right[i])
        {
          m_arrays_equal = false;
          CFdebug << m_prefix << "List mismatch at index "  << i << ": [" << left[i] << ", " << right[i] << "]" << CFendl;
        }
      }
    }

    // Floating point comparison
    template<typename NumberT>
    void compare_tables(boost::true_type, const Table<NumberT>& left, const Table<NumberT>& right)
    {
      m_arrays_equal = true;
      const Uint nb_rows = left.size();
      const Uint row_size = left.row_size();
      for(Uint i = 0; i != nb_rows; ++i)
      {
        const typename Table<NumberT>::ConstRow left_row = left[i];
        const typename Table<NumberT>::ConstRow right_row = right[i];
        for(Uint j = 0; j != row_size; ++j)
        {
          const Real diff = compare_numbers(left_row[j], right_row[j]);
          if(diff > m_max_ulps)
          {
            m_arrays_equal = false;
            CFdebug << m_prefix << "Array mismatch at index ["  << i << "," << j << "]: distance [" << left_row[j] << ", " << right_row[j] << "] is " << right_row[j]-left_row[j] << "(" << diff << " ulps)" << CFendl;
          }
        }
      }
    }

    // Integer number comparison
    template<typename NumberT>
    void compare_tables(boost::false_type, const Table<NumberT>& left, const Table<NumberT>& right)
    {
      m_arrays_equal = true;
      const Uint nb_rows = left.size();
      const Uint row_size = left.row_size();
      for(Uint i = 0; i != nb_rows; ++i)
      {
        const typename Table<NumberT>::ConstRow left_row = left[i];
        const typename Table<NumberT>::ConstRow right_row = right[i];
        for(Uint j = 0; j != row_size; ++j)
        {
          if(left_row[j] != right_row[j])
          {
            m_arrays_equal = false;
            CFdebug << m_prefix << "Array mismatch at index ["  << i << "," << j << "]: [" << left_row[j] << ", " << right_row[j] << "]" << CFendl;
          }
        }
      }
    }
    
    // compare floating point numbers
    template<typename T>
    Real compare_numbers(const T left, const T right)
    {
      // Don't try to compare zero using ULPs
      if(fabs(left) < m_zero_threshold && fabs(right) < m_zero_threshold)
      {
        return 0.;
      }
      
      return fabs(boost::math::float_distance(left, right));
    }
    
    Handle<Component const> m_left;
    Handle<Component const> m_right;
    const Uint m_max_ulps;
    const Real m_zero_threshold;
    
    bool& m_found_type;
    bool& m_arrays_equal;

    std::string m_prefix;
  };
}


  
ArrayDiff::ArrayDiff(const std::string& name): Action(name)
{
  options().add("left", Handle<Component const>())
    .pretty_name("Left")
    .description("Left array in the comparison")
    .mark_basic();
    
  options().add("right", Handle<Component const>())
    .pretty_name("Right")
    .description("Right array in the comparison")
    .mark_basic();
    
  options().add("max_ulps", 10u)
    .pretty_name("Max ULPs")
    .description("Maximum distance allowed between floating point numbers")
    .mark_basic();
    
  options().add("zero_threshold", 1e-16)
    .pretty_name("Zero Threshold")
    .description("Floating point numbers smaller than this are considered to be zero")
    .mark_basic();

  properties().add("arrays_equal", false);
}

ArrayDiff::~ArrayDiff()
{
}

void ArrayDiff::execute()
{
  typedef boost::mpl::vector5<int, Uint, float, double, bool> allowed_types;
  const Handle<Component const> left = options().value< Handle<Component const> >("left");
  const Handle<Component const> right = options().value< Handle<Component const> >("right");
  
  if(is_null(left))
    throw common::SetupError(FromHere(), "Left array is not set in " + uri().path());
  
  if(is_null(right))
    throw common::SetupError(FromHere(), "Right array is not set in " + uri().path());
  
  const Uint max_ulps = options().value<Uint>("max_ulps");
  const Real zero_threshold = options().value<Real>("zero_threshold");
  bool found_type = false;
  bool arrays_equal = false;
  boost::mpl::for_each<allowed_types>(detail::DoArrayDiff(left, right, max_ulps, zero_threshold, found_type, arrays_equal));
  if(!found_type)
    throw common::NotImplemented(FromHere(), "ArrayDiff does not support diff for type " + left->derived_type_name());

  CFdebug << "rank " << PE::Comm::instance().rank() << ": arrays " << left->uri().path() << " and " << right->uri().path();
  if(arrays_equal)
  {
    CFdebug << " are equal";
  }
  else
  {
    CFdebug << " differ";
  }
  CFdebug << CFendl;

  properties().set("arrays_equal", arrays_equal);
}

  
} // common
} // cf3
