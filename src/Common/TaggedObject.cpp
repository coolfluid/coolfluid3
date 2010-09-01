#include <boost/tokenizer.hpp>

#include "Common/TaggedObject.hpp"

using namespace CF::Common;

TaggedObject::TaggedObject() :
    m_tags(":") // empty tags
{

}

/////////////////////////////////////////////////////////////////////////////////////

void TaggedObject::add_tag(const std::string& tag)
{
  m_tags += tag + ":";
}

/////////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> TaggedObject::get_tags()
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
