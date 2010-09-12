//          Copyright Gunter Winkler 2004 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// authors: Gunter Winkler <guwi17 at gmx dot de>
//          Konstantin Kutzkow

#include <iostream>
#include <iomanip>
#include <boost/timer.hpp>

#include "lin_op.hpp"
#include "orthogonal.hpp"
#include "precond.hpp"


using namespace std;

int main(int argc, char *argv[])
{

  typedef boost::numeric::ublas::compressed_vector< double > cmp;
  typedef boost::numeric::ublas::compressed_matrix< double, boost::numeric::ublas::column_major > cmp_m;
  typedef boost::numeric::ublas::matrix< double, boost::numeric::ublas::column_major > mat_dense;
  typedef boost::numeric::ublas::banded_matrix< double, boost::numeric::ublas::column_major > band_matr;

  typedef boost::numeric::ublas::vector<double> Vector;
    
  int N = 200;  //the matrix size

  if (argc > 1) N = atoi(argv[1]);
  std::cout << N << std::endl;
    
  double tol = 1.0e-8; //the tolerance
    
  band_matr A(N, N, 1, 1);

  Vector exact(N);
  Vector b(N);
  Vector x(N);
    
  // define exact solution
  for (int i = 0; i < N; i ++ ) {
    exact(i) = 1;
  }

  double eps = 0.05; // set amount of asymmetry

  // prepare system matrix
  // matrix should be diagonal dominant to asure convergence ...
  for (unsigned int i = 0; i < A.size1(); ++i){
    A(i, i) = 2.1;
    if (i>0) A(i-1, i) = -1+eps;
    if (i+1<A.size1()) A(i+1, i)= -1-eps;
  }


  // compute rhs
  b = prod(A, exact);
  if (N<10) cout << "b = " << b << endl;


  // set up linear operator T
  LinOp< band_matr > T(A);

  // set up a simple preconditioner
  DiagonalPreconditioner< band_matr > prec(A);
    
  boost::timer t_gmres;
  unsigned int niter = 0;
    
  t_gmres.restart();
    
  // the test for the shortened version of the GMRes algorithm, the
  // search direction depends on the last 20 vectors of the
  // orthogonal basis

  // inital guess: solution = rhs
  x = b;

  // run GMRes and use \c mat_dense as type of temporary matrices
  niter = gmres_short< mat_dense >(T, x, b, prec, 20, 3*N, tol); 

  cout << "\nCPU time GMRES(short) = " << t_gmres.elapsed() << " (" << niter << ")" << endl;

  if (N < 10) cout << "x = " << x << endl;

  {
    // compute residuum to check solution quality
    Vector resid( (T.image_size()) );
    T.residuum(x,b,resid);
    cout <<"resid = " << boost::numeric::ublas::norm_2(resid) << endl;
  }

  t_gmres.restart();

  // the standard GMRes procedure with restarts after 20 steps, at most
  // 3 restarts are allowed
  x = b;
  niter = gmres_restarts< mat_dense >(T, x, b, prec, 20, 3*N/20, tol);
  cout << "\nCPU time GMRES  Restarts = " << t_gmres.elapsed() << " (" << niter << ")"  << endl;
  if (N < 10) cout << "x = " << x << endl;

  {
    Vector resid( (T.image_size()) );
    T.residuum(x,b,resid);
    cout <<"resid = " << boost::numeric::ublas::norm_2(resid) << endl;
  }
    
  t_gmres.restart();
    
  // the test for the general GMRes algorithm, we allow no restarts
  // but up to N iterations which guarantees convergence
  x = b;
  niter = gmres_restarts< mat_dense >(T, x, b, prec, N-1, 0, tol);
  cout << "\nCPU time GMRES = " << t_gmres.elapsed() <<" (" << niter << ")" << endl;
  if (N < 10) cout << "x = " << x << endl;

  {
    Vector resid( (T.image_size()) );
    T.residuum(x,b,resid);
    cout <<"resid = " << boost::numeric::ublas::norm_2(resid) << endl;
  }
    

  return EXIT_SUCCESS;
}
