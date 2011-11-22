// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Field_hpp
#define cf3_mesh_Field_hpp

#include "common/Table.hpp"

#include "mesh/SpaceFields.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Elements.hpp"

namespace cf3 {

namespace common
{
  class Link;
  namespace PE { class CommPattern; }
}
namespace math { class VariablesDescriptor; }


namespace mesh {

  class Region;

////////////////////////////////////////////////////////////////////////////////////////////

/// Field component class
/// This class stores fields which can be applied
/// to fields (Field)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API Field : public common::Table<Real> {

public: // typedefs




  enum VarType { SCALAR=1, VECTOR_2D=2, VECTOR_3D=3, TENSOR_2D=4, TENSOR_3D=9};

public: // functions

  /// Contructor
  /// @param name of the component
  Field ( const std::string& name );

  /// Virtual destructor
  virtual ~Field();

  /// Get the class name
  static std::string type_name () { return "Field"; }

  SpaceFields::Basis::Type basis() const { return m_basis; }

  void set_basis(const SpaceFields::Basis::Type basis) { m_basis = basis;}

  std::string var_name(Uint i=0) const;

  Uint nb_vars() const;

  /// True if the field contains a variable with the given name
  bool has_variable(const std::string& vname) const;

  /// Find the variable index of the given variable
  Uint var_number(const std::string& vname) const;

  /// Return the start index of a given variable
  Uint var_index(const std::string& vname) const;

  /// Return the start index of a given variable number
  Uint var_index(const Uint var_nb) const;

  /// Return the length (in number of Real values occupied in the data row) of the variable of the given name
  VarType var_length(const std::string& vname) const;

  /// Return the length (in number of Real values occupied in the data row) of the variable of the given var number
  VarType var_length(const Uint i=0) const;

  void set_topology(Region& topology);

  Region& topology() const;

  void set_field_group(SpaceFields& field_group);

  SpaceFields& field_group() const;

  virtual void resize(const Uint size);

  common::Table<Uint>::ConstRow indexes_for_element(const Entities& elements, const Uint idx) const;

  common::Table<Uint>::ConstRow indexes_for_element(const Uint unified_element_idx) const;

  common::List<Uint>& glb_idx() const { return field_group().glb_idx(); }

  common::List<Uint>& rank() const { return field_group().rank(); }

  bool is_ghost(const Uint idx) const { return field_group().is_ghost(idx); }

  const std::string& space() const { return field_group().space(); }

  Space& space(const Entities& entities) const { return entities.space(field_group().space()); }

  std::vector< Handle<Entities> > entities_range();

  std::vector< Handle<Elements> > elements_range();

  Field& coordinates() const { return field_group().coordinates(); }

  common::PE::CommPattern& parallelize_with( common::PE::CommPattern& comm_pattern );

  common::PE::CommPattern& parallelize();

  void synchronize();

  UnifiedData& elements_lookup() const { return field_group().elements_lookup(); }

  math::VariablesDescriptor& descriptor() const { return *m_descriptor; }

  void set_descriptor(math::VariablesDescriptor& descriptor);

  void create_descriptor(const std::string& description, const Uint dimension=0);


  ////////////////////////////////////////////////////////////////////////////////

    // Index operator.
    // --------------
    // c = U[i]
    // Real& operator[](int index);
    // const Real& operator[](int index) const;

    // // Binary arithmetic operators.
    // // ----------------------------
    // /// U = U + U
    // friend Field operator +(const Field& U1, const Field& U2);
    // /// U = U - U
    // friend Field operator -(const Field& U1, const Field& U2);
    // /// U = U * c
    // friend Field operator *(const Field& U, const Real& a);
    // /// U = c * U
    // friend Field operator *(const Real& a, const Field& U);
    // /// U = U * U (row-wise)
    // friend Field operator *(const Field& U1, const Field& U2);
    // /// U = U / c
    // friend Field operator /(const Field& U, const Real& a);
    // /// U = U / U (row-wise)
    // friend Field operator /(const Field& U1, const Field& U2);
    //
    // // Unary arithmetic operators.
    // // ---------------------------
    // /// U = -U
    // friend Field operator -(const Field& U);
    // //friend Field fabs(const Field& U);

    // Shortcut arithmetic operators.
    // ------------------------------
    /// U = U
    Field& operator =(const Field& U)
    {
      cf3_assert(size() == U.size());
      array() = U.array();
      return *this;
    }

    /// U = c
    Field& operator =(const Real& c)
    {
      for (Uint i=0; i<size(); ++i)
        for (Uint j=0; j<row_size(); ++j)
          array()[i][j] = c;
      return *this;
    }

    /// U += c
    Field& operator +=(const Real& c)
    {
      for (Uint i=0; i<size(); ++i)
        for (Uint j=0; j<row_size(); ++j)
          array()[i][j] += c;
      return *this;
    }

    /// U += U
    Field& operator +=(const Field& U)
    {
      cf3_assert(size() == U.size());
      cf3_assert(row_size() == U.row_size());
      for (Uint i=0; i<size(); ++i)
        for (Uint j=0; j<row_size(); ++j)
          array()[i][j] += U.array()[i][j];
      return *this;
    }

    /// U -= c
    Field& operator -=(const Real& c)
    {
      for (Uint i=0; i<size(); ++i)
        for (Uint j=0; j<row_size(); ++j)
          array()[i][j] -= c;
      return *this;
    }

    /// U -= U
    Field& operator -=(const Field& U)
    {
      cf3_assert(size() == U.size());
      cf3_assert(row_size() == U.row_size());
      for (Uint i=0; i<size(); ++i)
        for (Uint j=0; j<row_size(); ++j)
          array()[i][j] -= U.array()[i][j];
      return *this;
    }

    /// U *= c
    Field& operator *=(const Real& c)
    {
      for (Uint i=0; i<size(); ++i)
        for (Uint j=0; j<row_size(); ++j)
          array()[i][j] *= c;
      return *this;
    }

    /// U *= U
    Field& operator *=(const Field& U)
    {
      cf3_assert(size() == U.size());
      if (U.row_size() == 1) // U is a scalar field
      {
        for (Uint i=0; i<size(); ++i)
          for (Uint j=0; j<row_size(); ++j)
            array()[i][j] *= U.array()[i][0];
      }
      else
      {
        cf3_assert(row_size() == U.row_size()); // field must be same size
        for (Uint i=0; i<size(); ++i)
          for (Uint j=0; j<row_size(); ++j)
            array()[i][j] *= U.array()[i][j];
      }
      return *this;
    }

    /// U /= c
    Field& operator /=(const Real& c)
    {
      for (Uint i=0; i<size(); ++i)
        for (Uint j=0; j<row_size(); ++j)
          array()[i][j] /= c;
      return *this;
    }

    /// U /= U
    Field& operator /=(const Field& U)
    {
      cf3_assert(size() == U.size());
      if (U.row_size() == 1) // U is a scalar field
      {
        for (Uint i=0; i<size(); ++i)
          for (Uint j=0; j<row_size(); ++j)
            array()[i][j] /= U.array()[i][0];
      }
      else
      {
        cf3_assert(row_size() == U.row_size()); // field must be same size
        for (Uint i=0; i<size(); ++i)
          for (Uint j=0; j<row_size(); ++j)
            array()[i][j] /= U.array()[i][j];
      }
      return *this;
    }


    // // Relational operators.
    // // ---------------------
    // /// U == U
    // friend bool operator ==(const Field& U1, const Field& U2);
    // /// U != U
    // friend bool operator !=(const Field& U1, const Field& U2);
    // // /// U <= U
    // // friend bool operator <=(const Field& U1, const Field& U2);
    // // /// U >= U
    // // friend bool operator >=(const Field& U1, const Field& U2);
    // // /// U < U
    // // friend bool operator <(const Field& U1, const Field& U2);
    // // /// U > U
    // // friend bool operator >(const Field& U1, const Field& U2);
    //
    // // Input-output operators.
    // // ----------------------
    // /// ostream << U
    // friend ostream& operator << (ostream& out, const Field& U);
    // /// istream >> U
    // friend istream& operator >> (istream& in,  Field& U);




private:

  void config_var_names();
  void config_var_types();

  SpaceFields::Basis::Type m_basis;
  Handle<Region> m_topology;
  Handle<SpaceFields> m_field_group;

  Handle< common::PE::CommPattern > m_comm_pattern;

  Handle< math::VariablesDescriptor > m_descriptor;
};

////////////////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#endif // cf3_mesh_Field_hpp
