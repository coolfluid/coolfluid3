#ifndef CF_Common_TaggedObject_hpp
#define CF_Common_TaggedObject_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  //////////////////////////////////////////////////////////////////////////

  /// Manages tags
  class Common_API TaggedObject
  {
  public:

    /// Constructor
    TaggedObject();

    /// Check if this component has a given tag assigned
    bool has_tag(const std::string& tag) const;

    /// add tag to this component
    void add_tag(const std::string& tag);

    /// @return tags in a vector
    std::vector<std::string> get_tags();

  private:

    std::string m_tags;

  }; // class TaggedObject

  //////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_TaggedObject_hpp
