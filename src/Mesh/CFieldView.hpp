// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CFieldView_hpp
#define CF_Mesh_CFieldView_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

  template <typename T> class CList;
  class CEntities;
  class CField;
  class CSpace;
  class CFaceCellConnectivity;
  class CMeshElements;

////////////////////////////////////////////////////////////////////////////////

class Mesh_API CFieldView : public Common::Component
{
public: // typedefs

  typedef boost::shared_ptr<CFieldView> Ptr;
  typedef boost::shared_ptr<CFieldView const> ConstPtr;

  typedef CTable<Real>::Row View;
  typedef CTable<Real>::ConstRow ConstView;

protected: // typedefs

  typedef boost::multi_array_types::index_range Range;

public: // functions

  /// Contructor
  /// @param name of the component
  CFieldView ( const std::string& name );

  /// Virtual destructor
  virtual ~CFieldView() {}

  /// Get the class name
  static std::string type_name () { return "CFieldView"; }

  /// @return end_idx
  Uint initialize(CField& field, boost::shared_ptr<CEntities> elements);

  CTable<Real>::Row operator[](const Uint idx);

  CTable<Real>::ConstRow operator[](const Uint idx) const;

  const CField& field() const { return *m_field.lock(); }

  CField& field() { return *m_field.lock(); }

  const CEntities& elements() const { return *m_elements.lock(); }

  Uint stride() const { return m_stride; }

  Uint size() const { return m_size; }

  /// @return elements_exist_in_field
  virtual bool set_elements(const CEntities& elements);

  /// @return elements_exist_in_field
  bool set_elements(boost::shared_ptr<CEntities> elements);

  void set_field(CField& field);
  void set_field(const CField& field);
  void set_field(boost::shared_ptr<CField> field);

  const CSpace& space() const { cf_assert(m_space.expired()==false); return *m_space.lock(); }

  Uint mesh_elements_idx(const Uint idx) const;

protected:

  Uint m_start_idx;
  Uint m_end_idx;
  Uint m_stride;
  Uint m_size;

  boost::weak_ptr<CField>               m_field;
  boost::weak_ptr<CTable<Real> >        m_field_data;
  boost::weak_ptr<CEntities const>      m_elements;
  boost::weak_ptr<CTable<Real> const>   m_coords_table;
  boost::weak_ptr<CSpace const>         m_space;
  boost::weak_ptr<CMeshElements const>  m_mesh_elements;
};

////////////////////////////////////////////////////////////////////////////////

class Mesh_API CMultiStateFieldView : public CFieldView
{
public: // typedefs

  typedef boost::shared_ptr<CMultiStateFieldView> Ptr;
  typedef boost::shared_ptr<CMultiStateFieldView const> ConstPtr;

  typedef CTable<Real>::ArrayT::array_view<2>::type View;
  typedef const CTable<Real>::ArrayT::const_array_view<2>::type ConstView;

public: // functions

  /// Contructor
  /// @param name of the component
  CMultiStateFieldView ( const std::string& name );

  /// Virtual destructor
  virtual ~CMultiStateFieldView() {}

  /// Get the class name
  static std::string type_name () { return "CMultiStateFieldView"; }

  View operator[](const Uint idx);

  ConstView operator[](const Uint idx) const;

};

////////////////////////////////////////////////////////////////////////////////

class Mesh_API CScalarFieldView : public CFieldView
{
public: // typedefs

  typedef boost::shared_ptr<CScalarFieldView> Ptr;
  typedef boost::shared_ptr<CScalarFieldView const> ConstPtr;

  typedef Real& View;
  typedef const Real& ConstView;

public: // functions

  /// Contructor
  /// @param name of the component
  CScalarFieldView ( const std::string& name );

  /// Virtual destructor
  virtual ~CScalarFieldView() {}

  /// Get the class name
  static std::string type_name () { return "CScalarFieldView"; }

  Real& operator[](const Uint idx);

  const Real& operator[](const Uint idx) const;

};

////////////////////////////////////////////////////////////////////////////////

class Mesh_API CConnectedFieldView : public Common::Component
{
public:
  /// Contructor
  /// @param name of the component
  CConnectedFieldView ( const std::string& name ) : Common::Component(name) {}

  /// Virtual destructor
  virtual ~CConnectedFieldView() {}

  /// Get the class name
  static std::string type_name () { return "CConnectedFieldView"; }

  void initialize(boost::shared_ptr<CField> field, boost::shared_ptr<CEntities> faces);

  void set_field(boost::shared_ptr<CField> field);

  void set_field(CField& field);

  bool set_elements(boost::shared_ptr<CEntities> elements);

  bool set_elements(CEntities& elements);

  std::vector<CTable<Real>::Row> operator[](const Uint elem_idx);

  CTable<Real>::Row operator()(const Uint elem_idx, const Uint connected_idx);

  Mesh::CField& field();

  CTable<Uint>::ConstRow connected_idx(const Uint idx) const;

private:

  boost::weak_ptr<CEntities> m_elements;
  boost::weak_ptr<CFaceCellConnectivity> m_face2cells;
  std::vector<CFieldView::Ptr> m_views;

  boost::weak_ptr<CField> m_field;

  Uint cells_comp_idx;
  Uint cell_idx;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CFieldView_hpp
