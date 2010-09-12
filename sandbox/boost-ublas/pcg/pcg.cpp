/** -*- c++ -*- \file pcg_test.hpp \brief demo: solve a linear system using CG */
/*
 -   begin                : 2005-08-24
 -   copyright            : (C) 2005 by Gunter Winkler, Konstantin Kutzkow
 -   email                : guwi17@gmx.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <cassert>
#include <limits>

#include <boost/timer.hpp>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>

#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/banded.hpp>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>

#include <boost/numeric/ublas/io.hpp>

#include "precond.hpp"
#include "pcg.hpp"

/** \brief fill lower triangular matrix L 
 */
template < class MATRIX >
void fill_symm(MATRIX & L, const size_t bands = std::numeric_limits<size_t>::max() )
{
  typedef typename MATRIX::size_type size_type;
  
  assert(L.size1() == L.size2());

  size_type size = L.size1();
  for (size_type i=0; i<size; i++) {
    for (size_type j = ((i>bands)?(i-bands):0); j<i; j++) {
      if ( ( ((int)(11*sin(3.0*i+5.0*j))) % 7 ) > 3 || ( abs(i-j) <= 2 ) )
        L(i,j) = 1 + (1.0 + (double) j)/(1.0 + (double) i) + 1.0/(1.0 + j);
    }
    L(i,i) = 1 + 2 * i ;
  }

  return;
}


template < class PRECOND, class MATRIX, class VECTOR >
void run_solve(const MATRIX& A, const VECTOR& b, const VECTOR& y)
{
  typedef typename MATRIX::value_type  DBL;

  boost::timer  t1;

  double prec, sv;

  t1.restart();
  PRECOND precond(A);
  prec = t1.elapsed();    
  std::cout << " (precond: " << prec << " sec)" << std::flush;

  t1.restart();
  // initial guess
  ublas::vector<DBL> x = b;
  // solve
  size_t res = pcg_solve(A, x, b, 
                         precond, 
                         5000,
                         1e-10,
                         1e-20);
  sv = t1.elapsed();
    
  std::cout << " (niter: " << res << ") " 
            << ublas::norm_2(x-y)/ublas::norm_2(y) << " "
//            << ublas::norm_2(prod(A,x)-b)/ublas::norm_2(b) << " "
            << " (solve: " << sv << " sec)"
            << std::flush;
}

template < class MATRIX >
void run_test(size_t size)
{
  typedef typename MATRIX::value_type  DBL;

  boost::timer  t1;
  double pr;

  MATRIX A (size, size);
  MATRIX T (size, size);

  ublas::vector<DBL> b (size);
  ublas::vector<DBL> x (size);
  ublas::vector<DBL> y (size);

  std::fill(y.begin(), y.end(), 1.0);

//   A = ublas::zero_matrix<DBL>(size, size);
  A.clear();

  std::cout << size << ": " << std::flush;

  fill_symm(T);
  t1.restart();
  A = ublas::prod(T, trans(T));
  pr = t1.elapsed();
  std::cout << " (prod: " << pr << " sec)" << std::flush;

  b = prod(A, y);

  std::cout << "\n    id:";
  run_solve< IdentityPreconditioner<MATRIX> >(A, b, y);

  std::cout << "\n  diag:";
  run_solve< DiagonalPreconditioner<MATRIX> >(A, b, y);

  std::cout << "\n  chol:";
  run_solve< CholeskyPreconditioner<MATRIX> >(A, b, y);

  // works only for sparse types
  std::cout << "\n    ic:";
  run_solve< IncompleteCholeskyPreconditioner<MATRIX> >(A, b, y);

  std::cout << std::endl;
}


int main(int argc, char * argv[] )
{
  size_t size = 10;
  if (argc > 1)
    size = ::atoi (argv [1]);

  typedef double DBL;
  typedef ublas::row_major ORI;

//   run_test< ublas::matrix<DBL, ORI> >(size);

  run_test< ublas::compressed_matrix<DBL, ORI> >(size);

  return EXIT_SUCCESS;
}
