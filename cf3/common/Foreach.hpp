// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_FOREACH_hpp
#define cf3_common_FOREACH_hpp

#include <boost/foreach.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>

/// lowercase version of BOOST_FOREACH
#define boost_foreach BOOST_FOREACH 

// Following is a feature request in boost
// It will be used for access to container variables:
//
// example 1:
// std::map<int, int> my_map;
// foreach_container((int key)(int value), my_map)
//   std::cout << key << " : " << value << "\n";
//
// example 2: 
// std::vector<boost::fusion::vector<int, std::string, int> > my_complex_type;
// foreach_container((int i)(std::string const& j)(int& k), my_complex_type)
//   std::cout << "line: " << i << ", " << j << ", " << ++k;

#define BOOST_FOREACH_ASSIGN_VAR(R, ROW, I, VAR)                                                                  \
  for (VAR = boost::fusion::at_c<I>(ROW); !BOOST_FOREACH_ID(_foreach_leave_outerloop);                            \
      BOOST_FOREACH_ID(_foreach_leave_outerloop) = true)

/// boost_foreach version that allows to use std::map's, boost::tuple's, std::pair's internal variables
#define foreach_container(VARS, COL)                                                                                  \
    BOOST_FOREACH_PREAMBLE()                                                                                      \
    if (boost::foreach_detail_::auto_any_t BOOST_FOREACH_ID(_foreach_col) = BOOST_FOREACH_CONTAIN(COL)) {} else   \
    if (boost::foreach_detail_::auto_any_t BOOST_FOREACH_ID(_foreach_cur) = BOOST_FOREACH_BEGIN(COL)) {} else     \
    if (boost::foreach_detail_::auto_any_t BOOST_FOREACH_ID(_foreach_end) = BOOST_FOREACH_END(COL)) {} else       \
    for (bool BOOST_FOREACH_ID(_foreach_continue) = true,                                                         \
        BOOST_FOREACH_ID(_foreach_leave_outerloop) = true;                                                        \
        BOOST_FOREACH_ID(_foreach_continue) && !BOOST_FOREACH_DONE(COL);                                          \
        BOOST_FOREACH_ID(_foreach_continue) ? BOOST_FOREACH_NEXT(COL) : (void)0)                                  \
        if  (boost::foreach_detail_::set_false(BOOST_FOREACH_ID(_foreach_continue))) {} else                      \
        if  (boost::foreach_detail_::set_false(BOOST_FOREACH_ID(_foreach_leave_outerloop))) {} else               \
        BOOST_PP_SEQ_FOR_EACH_I(BOOST_FOREACH_ASSIGN_VAR, BOOST_FOREACH_DEREF(COL), VARS)                         \
        for (; !BOOST_FOREACH_ID(_foreach_continue); BOOST_FOREACH_ID(_foreach_continue) = true)


#endif // cf3_common_FOREACH_hpp
