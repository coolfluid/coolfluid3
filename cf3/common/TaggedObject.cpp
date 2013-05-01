// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/tokenizer.hpp>

#include "common/TaggedObject.hpp"

using namespace cf3::common;

TaggedObject::TaggedObject() :
    m_tags(":") // empty tags
{
}

/////////////////////////////////////////////////////////////////////////////////////

void TaggedObject::add_tag(const std::string& tag)
{
  if (!has_tag(tag))
    m_tags += tag + ":";
}

/////////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> TaggedObject::get_tags() const
{
  std::vector<std::string> vec;

  typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
  boost::char_separator<char> sep(":");
  Tokenizer tokens(m_tags, sep);

  for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
    vec.push_back(*tok_iter);

  return vec;
}

/////////////////////////////////////////////////////////////////////////////////////

bool TaggedObject::has_tag(const std::string& tag) const
{
  typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
  boost::char_separator<char> sep(":");
  Tokenizer tokens(m_tags, sep);

  for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
    if (*tok_iter == tag)
      return true;

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////

void TaggedObject::remove_tag(const std::string& tag)
{
  if (has_tag(tag))
  {
    std::string tags;

    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
    boost::char_separator<char> sep(":");
    Tokenizer tokens(m_tags, sep);

    for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
      if (*tok_iter!=tag)
        tags += *tok_iter + ":";
    m_tags=tags;
  }
}
