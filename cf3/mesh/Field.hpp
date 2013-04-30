// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Field_hpp
#define cf3_mesh_Field_hpp

#include "common/Table.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Elements.hpp"

namespace cf3 {

namespace common {
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

  typedef ArrayT::array_view<2>::type View;


  typedef Eigen::Array<Real,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> ArrayStorage ;
  typedef Eigen::Map< ArrayStorage , Eigen::Unaligned, Eigen::Stride<Eigen::Dynamic,Eigen::Dynamic> > Ref ;

  typedef Eigen::Block<Ref, Eigen::Dynamic, 1, false, true> RefCol;

  typedef Eigen::Array<Real,1,Eigen::Dynamic,Eigen::RowMajor> RowArrayStorage ;
  typedef Eigen::Map< RowArrayStorage , Eigen::Unaligned > RowArrayRef ;

  typedef Eigen::Matrix<Real,1,Eigen::Dynamic,Eigen::RowMajor> RowVectorStorage ;
  typedef Eigen::Map< RowVectorStorage , Eigen::Unaligned > RowVectorRef ;

  typedef Eigen::Matrix<Real,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> RowTensorStorage ;
  typedef Eigen::Map< RowTensorStorage , Eigen::Unaligned > RowTensorRef ;

private: // typedefs

  typedef boost::multi_array_types::index_range range;

public: // functions

  /// Contructor
  /// @param name of the component
  Field ( const std::string& name );

  /// Virtual destructor
  virtual ~Field();

  /// Get the class name
  static std::string type_name () { return "Field"; }

  std::string var_name(Uint i=0) const;

  Uint nb_vars() const;

  /// True if the field contains a variable with the given name
  bool has_variable(const std::string& vname) const;

  /// Find the variable index of the given variable
  Uint var_number(const std::string& vname) const;

  /// Return the start index of a given variable
  Uint var_offset(const std::string& vname) const;

  /// Return the start index of a given variable number
  Uint var_offset(const Uint var_nb) const;

  /// Return the length (in number of Real values occupied in the data row) of the variable of the given name
  VarType var_length(const std::string& vname) const;

  /// Return the length (in number of Real values occupied in the data row) of the variable of the given var number
  VarType var_length(const Uint i=0) const;

  void set_dict(Dictionary& dict);

  Dictionary& dict() const;

  View view(const Uint start, const Uint size);

  View view(common::Table<Uint>::ConstRow& indices);

  common::List<Uint>& glb_idx() const;

  common::List<Uint>& rank() const;

  bool is_ghost(const Uint idx) const;

  bool continuous() const;

  bool discontinuous() const;

  const Handle<Space const>& space(const Handle<Entities const>& entities) const;

  const Space& space(const Entities& entities) const;

  const std::vector< Handle<Entities> >& entities_range() const;

  const std::vector< Handle<Space> >& spaces() const;

  Field& coordinates() const;

  common::PE::CommPattern& parallelize_with( common::PE::CommPattern& comm_pattern );

  common::PE::CommPattern& parallelize();

  void synchronize();

  math::VariablesDescriptor& descriptor() const { return *m_descriptor; }

  void set_descriptor(math::VariablesDescriptor& descriptor);

  void create_descriptor(const std::string& description, const Uint dimension=0);

  Ref ref();

  Ref col(const Uint c);

  RowArrayRef row(const Uint r);

  Real& scalar(const Uint r);

  RowVectorRef vector(const Uint r);

  RowTensorRef tensor(const Uint r);

  void set_var_type(const VarType var_type);

  VarType var_type() const ;

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

  Handle<Dictionary> m_dict;

  Handle< common::PE::CommPattern > m_comm_pattern;

  Handle< math::VariablesDescriptor > m_descriptor;

  VarType m_var_type;
};

////////////////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#endif // cf3_mesh_Field_hpp
