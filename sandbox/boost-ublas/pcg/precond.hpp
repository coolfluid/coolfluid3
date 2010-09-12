/** -*- c++ -*- \file precond.hpp \brief define preconditioners */

//          Copyright Gunter Winkler 2004 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// authors: Gunter Winkler <guwi17 at gmx dot de>


#ifndef _H_PRECOND_HPP_
#define _H_PRECOND_HPP_

#include <iostream>

#include <boost/numeric/ublas/vector_expression.hpp>

#include "cholesky.hpp"

namespace ublas = boost::numeric::ublas;

/** \brief Do-nothing preconditioner
 */
template < class MATRIX = int >
class IdentityPreconditioner {
  
public:
  IdentityPreconditioner(const MATRIX& A) { }
  
  template<class Vector>
  void apply(const Vector& b, Vector& x) const
  {
    x = b;
  }
private:
  IdentityPreconditioner() { }
};

/** \brief scales x with diagonal of a given matrix
 */
template<class Matrix> 
class DiagonalPreconditioner {

public:
  typedef typename Matrix::value_type value_type;
  typedef typename Matrix::size_type size_type;

  DiagonalPreconditioner(const Matrix & A, const double eps = 1.0e-14) 
    : diag(A.size1()), EPS(eps)
  { 
    for (size_type i=0; i<A.size1(); ++i) {
      if ( fabs( A(i,i) ) > EPS ) {
        diag(i) = 1.0 / A(i,i);
      } else {
        diag(i) = 0;
      }
    }
  }
  
  template<class Vector>
  void apply(const Vector& b, Vector& x) const
  {
    x = element_prod( diag, b );
  }

private:
  DiagonalPreconditioner() { };
  ublas::vector< value_type > diag;
  const double EPS;
  
};

template <class Matrix>
DiagonalPreconditioner<Matrix>
make_DiagonalPreconditioner(const Matrix & A, const double eps = 1.0e-14)
{
  return DiagonalPreconditioner<Matrix>(A, eps);
}


/** \brief decomposes given matrix and solve L L^T x = b
 *
 * note: This is actually a Cholesky solver, not a real preconditioner
 */
template<class Matrix> class CholeskyPreconditioner {

public:
  CholeskyPreconditioner(const Matrix & A) : L(A) 
  { 
    cholesky_decompose(L);
  }
  
  template<class Vector>
  void apply(const Vector& b, Vector& x) const
  {
    x = b;
    cholesky_solve(L, x, ublas::lower());
  }

private:
  CholeskyPreconditioner() { }
  Matrix L;
};


template <class Matrix>
CholeskyPreconditioner<Matrix>
make_CholeskyPreconditioner(const Matrix & A)
{
  return CholeskyPreconditioner<Matrix>(A);
}



/** \brief decomposes given matrix and solve L L^T x = b
 *
 * note: L is a lower triangular matrix computed by a fast 
 *       incomplete cholesky decomposition. L has the same
 *       sparsity pattern as A and a full diagonal.
 */
template<class Matrix> class IncompleteCholeskyPreconditioner {

public:
  IncompleteCholeskyPreconditioner(const Matrix & A) : L(A) 
  { 
    incomplete_cholesky_decompose(L);
  }
  
  template<class Vector>
  void apply(const Vector& b, Vector& x) const
  {
    x = b;
    cholesky_solve(L, x, ublas::lower());
  }

private:
  IncompleteCholeskyPreconditioner() { }
  Matrix L;
};


template <class Matrix>
IncompleteCholeskyPreconditioner<Matrix>
make_IncompleteCholeskyPreconditioner(const Matrix & A)
{
  return IncompleteCholeskyPreconditioner<Matrix>(A);
}


#endif
