#ifndef CF_Common_ConnectivityTableCF2_hh
#define CF_Common_ConnectivityTableCF2_hh

////////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <valarray>
#include <limits>

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Mesh {

    template <class T> class ConnectivityTableCF2;

    template <class T> std::ostream& operator<< (std::ostream& out, const ConnectivityTableCF2<T>& A);

    template <class T> std::istream& operator>> (std::istream& in, ConnectivityTableCF2<T>& A);

////////////////////////////////////////////////////////////////////////////////

/// This class provides a generic table with arbitrary number
/// of columns for its rows.
/// If the data member m_isHybrid is true the table has the
/// logical number of columns equal to the maximum number of
/// columns and m_columnPattern keeps knowledge of the actual size and
/// m_columnPattern.size() == _mrowSize
/// If the storage is not hybrid then m_columnPattern.size() == 1.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class T>
class ConnectivityTableCF2 {
public: // friend operators

  /// Overloading of the stream operator "<<" for the output to screen
  friend std::ostream& operator<< (std::ostream& out, const ConnectivityTableCF2<T>& A)
  {
    for (Uint i = 0; i < A.nbRows(); ++i) {
      const Uint nbcols = A.nbCols(i);
      for (Uint j = 0; j < nbcols; ++j) {
        out << A(i,j) << " " ;
      }
      out << "\n";
    }
    return out;
  }

  /// Overloading of the stream operator ">>" for the input from screen
  friend std::istream& operator>> (std::istream& in, ConnectivityTableCF2<T>& A)
  {
    for (Uint i = 0; i < A.nbRows(); ++i) {
      const Uint nbcols = A.nbCols(i);
      for (Uint j = 0; j < nbcols; ++j) {
        in >> A(i,j);
      }
    }
    return in;
  }

public: // methods

  /// Default constructor
  ConnectivityTableCF2() :  m_nbentries(0), m_nbrows(0), m_nbcols(0), m_table(0)
  {
  }

  /// Constructor
  /// @param columnPattern gives the number of columns per row
  /// @param value         initializing value
  ConnectivityTableCF2(const std::valarray<Uint>& columnPattern, T value = T())
  {
    deallocate();
    const Uint maxCol = findMaxCol(columnPattern);
    allocate(columnPattern.size(),maxCol);
    putPattern(columnPattern,value);
  }

  /// Copy Constructor
  ConnectivityTableCF2(const ConnectivityTableCF2<T>& init)
  {
    this->operator=(init);
  }

  /// Destructor
  ~ConnectivityTableCF2()
  {
    deallocate();
  }

  /// Get the total number of entries in the table
  Uint size() const
  {
    cf_assert(m_nbentries <= m_nbrows*m_nbcols);
    return m_nbentries;
  }

  /// Resize the table
  /// @param columnPattern gives the number of columns per row
  /// @param value         initializing value
  void resize(const std::valarray<Uint>& columnPattern, T value = T())
  {
    deallocate();
    const Uint maxCol = findMaxCol(columnPattern);
    allocate(columnPattern.size(),maxCol);
    putPattern(columnPattern,value);
  }

  /// Clear the table
  void clear()
  {
    deallocate();
  }

  /// Assignment operator
  const ConnectivityTableCF2<T>& operator= (const ConnectivityTableCF2<T>& other)
  {
    deallocate();
    allocate(other.m_nbrows,other.m_nbcols);
    copyTable(other.m_table);
    m_nbentries = other.m_nbentries;
    return *this;
  }

  /// Mutator for table elements
  T& operator() (Uint iRow, Uint jCol)
  {
    cf_assert(iRow < m_nbrows);
    cf_assert(jCol < nbCols(iRow));
    return m_table[jCol*m_nbrows + iRow];
  }

  /// Accessor for table elements
  T operator() (Uint iRow, Uint jCol) const
  {
    cf_assert(m_table.size() > 0);
    cf_assert(iRow < m_nbrows);
    cf_assert(jCol < nbCols(iRow));
    return m_table[jCol*m_nbrows + iRow];
  }

  /// Get the number of rows
  Uint nbRows() const {return m_nbrows;}

  /// Get the number of columns
  Uint nbCols(Uint iRow) const
  {
    ///@todo change this
    cf_assert(iRow < m_nbrows);
    Uint nbcol = m_nbcols;
    for (; nbcol >= 1; --nbcol) {
      if (m_table[(nbcol-1)*m_nbrows + iRow] != NOVALUE) return nbcol;
    }
    return 0;
  }

  /// Tell if the table is hybrid
  bool isHybrid() const {return true;}

  /// Tell if the table is not hybrid
  bool isNotHybrid() const {return false;}

  /// Set the row to the given values
  void setRow(Uint iRow, std::vector<Uint>& row) const
  {
    const Uint nbC = nbCols(iRow);
    cf_assert(row.size() <= m_nbcols);
    for (Uint jCol = 0; jCol < nbC; ++jCol) {
      row[jCol] = (*this)(iRow,jCol);
    }
  }

  /// Overloading of the operator "-=",
  /// allowing to subtract the same value to all
  /// the elements of the table
  /// @param scalar value to subtract
  /// @return a reference to the modified table
#define CONN_TABLE_OP(__op__)                                 \
  const ConnectivityTableCF2<T>& operator __op__(const T& value) \
  {                                                           \
    for (Uint iRow = 0; iRow < m_nbrows; ++iRow) {           \
      for (Uint jCol = 0 ; jCol < nbCols(iRow); ++jCol) {     \
        (*this)(iRow,jCol) __op__ value;                   \
      }                                                       \
    }                                                         \
    return *this;                                             \
  }

  CONN_TABLE_OP(=)
  CONN_TABLE_OP(+=)
  CONN_TABLE_OP(-=)
  CONN_TABLE_OP(*=)

#undef CONN_TABLE_OP

private: // helper functions

  /// Allocate
  void allocate(Uint nbrows, Uint nbcols)
  {
    m_nbrows = nbrows;
    m_nbcols = nbcols;
    m_table.resize(nbrows*nbcols);
  }

  /// Deallocate
  void deallocate()
  {
    m_nbentries = 0;
    m_nbrows = 0;
    m_nbcols = 0;
    if (m_table.size() != 0) {
      std::vector<T>().swap(m_table);
    }
    cf_assert(m_table.capacity() == 0);
    cf_assert(m_table.size() == 0);
  }

  /// Find max number of columns
  Uint findMaxCol(const std::valarray<Uint>& columnPattern)
  {
    Uint maxCols = 0;
    for (Uint iRow = 0; iRow < columnPattern.size(); ++iRow) {
      if(maxCols < columnPattern[iRow]) maxCols = columnPattern[iRow];
    }
    return maxCols;
  }

  /// Put the pattern in the table
  void putPattern(const std::valarray<Uint>& columnPattern, const T& value)
  {
    m_nbentries = 0;
    for (Uint iRow = 0; iRow < m_nbrows; ++iRow)
    {
      // update the number of entries
      m_nbentries += columnPattern[iRow];

      for (Uint jCol = 0; jCol < m_nbcols; ++jCol)
      {
        if (jCol < columnPattern[iRow]) {
          (*this)(iRow,jCol) = value;
        }
        else {
          (*this)(iRow,jCol) = NOVALUE;
        }
      }
    }
    cf_assert(m_nbentries <= m_nbrows*m_nbcols);
  }

  /// Elementwise copy of the table elements
  void copyTable(const std::vector<T>& other)
  {
    cf_assert(m_table.size() == other.size());
    const Uint tsize = m_nbrows*m_nbcols;
    for (Uint i = 0; i < tsize; ++i) {
      m_table[i] = other[i];
    }
  }

private: // data

  /// number of entries in the table, less or equal to m_nbrows*m_nbcols
  Uint m_nbentries;
  /// row size
  Uint m_nbrows;
  /// maximum column size
  Uint m_nbcols;
  /// the actual storage of the table
  std::vector<T> m_table;
  /// this value means this place in memory shouldn't be used
  static const T NOVALUE;

}; // end of class ConnectivityTableCF2

////////////////////////////////////////////////////////////////////////////////

template <typename T>
const T ConnectivityTableCF2<T>::NOVALUE = std::numeric_limits<T>::max();

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ConnectivityTableCF2_hh
