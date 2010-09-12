/** \file orthogonal.hpp \brief The gmres algorithm. -*- c++ -*- */

//          Copyright Gunter Winkler 2004 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// authors: Gunter Winkler <guwi17 at gmx dot de>
//          Konstantin Kutzkow


#ifndef __INCLUDE_ORTHOGONAL_HPP__
#define __INCLUDE_ORTHOGONAL_HPP__

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/vector_of_vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/operation_sparse.hpp>

#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/banded.hpp>
#include <boost/numeric/ublas/io.hpp>


using namespace std;
namespace ublas = boost::numeric::ublas;


/** \brief returns the orthogonal system of a given set of vectors
* - \param Vector is the type of the vectors
*/
template <class Vector>
std::vector< Vector >  orthogonal_system(std::vector< Vector > & basis){

    ublas::matrix<double> r (basis.size(), basis.size());
    std::vector< Vector > q(basis.size());

     for (int i = 0; i < basis.size(); ++i){

            r(i, i) = ublas::norm_2(basis[i]);
            q[i] = basis[i]/r(i, i);

            for (int j = i+1; j < basis.size(); ++j){
                r(i, j) = ublas::inner_prod(q[i], basis[j]);
                basis[j] -= r(i, j)*q[i];
            }

        }
        
    return q;
}

/** \brief preforms the orthogonalization of a given vector to a given basis using the arnoldi approach
* - \param Vector is the type of the vectors
*/
template <class Vector>
Vector orthogonal_vector(std::vector< Vector > & basis, Vector & v){

    double r = 0;
    
    for (int i = 0; i < basis.size(); ++i){
        r = ublas::inner_prod(basis[i], v);
        v -= r*basis[i];
    }
    return v/ublas::norm_2(v);
}

/** \brief preforms the orthogonalization of a given vector to a given basis using the arnoldi approach
* the basis is represented as a matrix
* - \param Vector is the type of the vectors
* - \param Matrix is the type of the matrix
*/
template <class Matrix, class Vector>
Vector arnoldi_step (std::vector< Vector > & q, Matrix & A, Matrix & H, int n){
    Vector v = ublas::prod(A, q[n]);

    for (int j = 0; j <= n; ++j){
        H(j, n) = ublas::inner_prod(q[j], v);
        v -= H(j, n)*q[j];
    }
    H(n+1, n) = ublas::norm_2(v);
    q[n+1] = v/H(n+1, n);
    
    return q[n+1];

}

/** \brief the standard gmres algorithm, without linear operator or any improvements
* - \param Vector is the type of the vectors
* - \param Matrix is the type of the matrix
* - \note the input parameter x must be predefined
*/
template <class Matrix, class Vector>
void gmres (const Matrix & A, Vector & x, const Vector & b, double tol){

    const std::size_t image_size = A.image_size();
    const std::size_t preimage_size = A.preimage_size();

    Matrix  Q(image_size, preimage_size+1);
    Matrix  H(image_size, preimage_size);
    
    
    Vector v(image_size);
    Vector c(image_size+1);
    Vector s(image_size+1);
    Vector z(image_size+1);
    
   
    v = b - prod(A, x);
    column(Q, 0) = v/ublas::norm_2(v);
    
    z(0) = ublas::norm_2(v);
    
    double z_0 = z(0);
    
    double alpha = 0;
    
    double t = tol + 1;
    
    int k = 0;


    while (k < image_size && t > tol){
        v = ublas::prod(A, column(Q, k));
       
        Vector h = column(H, k);
        
        Vector q;

        for (int j = 0; j <= k; ++j){
            q = column(Q, j);
            h(j) = ublas::inner_prod(q, v);
            v -=  h(j)*q;
        }

        h(k+1) = ublas::norm_2(v);
        column(Q, k+1) = v/h(k+1);
        

        double h_i = 0;
        
        for (int i = 0; i < k; ++i){
            h_i = h(i);
            h(i) = c(i+1)*h(i) + s(i+1)*h(i+1);
            h(i+1) = s(i+1)*h_i - c(i+1)*h(i+1);
            }

        double h_kk = h(k);
        double h_k1k = h(k+1);
        alpha  = sqrt(h_kk*h_kk + h_k1k*h_k1k);
        s(k+1) = h_k1k/alpha;
        c(k+1) = h_kk/alpha;
        h(k) = alpha;

        column(H, k) = h;
        
        z(k+1) = s(k+1)*z(k);
        z(k) = c(k+1)*z(k);
        
        
        t=  z(k+1)/z_0;
        ++k;

        if (0) {
            Vector s(x);
            Vector c(k+1);
            for (int i = k; i >= 0; --i)
                c(i) = (z(i) - inner_prod(project(row(H, i), ublas::range(i+1, k+1) ), project( c, ublas::range(i+1, k+1) )))/H(i, i);
            for (int i = 0; i < k; ++i)
                s += c(i)*column(Q, i);
            cout << norm_2(prod(A,s)-b) << endl;
        }

    }

    
    c.clear();
    
    for (int i = k; i >= 0; --i)
            c(i) = (z(i) - inner_prod(project(row(H, i), ublas::range(i+1, k+1) ), project( c, ublas::range(i+1, k+1) )))/H(i, i);
    
    
    for (int i = 0; i < k; ++i)
        x += c(i)*column(Q, i);

}

/** \brief a single step of the gmres algorithm, one iteration. Returns true if the approach converges.
* - \param LinOp is the type of the linear opeartor representing the matrix
* - \param Vector is the type of the vectors
* - \param Matrix is the type of the matrix
* - \param Precond is the type of the used preconditioner
* - \note the input parameter x must be predefined, 
*   assume maxiter < A.image_size()
*/
template <class Matrix, class LinOp, class Vector, class Precond>
unsigned int gmres_step ( LinOp const & A, 
                          Vector & x, 
                          const Vector & b, 
                          Precond const & B, 
                          unsigned int const maxiter, 
                          double const tol) 
{

    unsigned int size = A.image_size();

    Matrix  Q(size, maxiter + 1);
    Matrix  H(size, maxiter + 1);
    
    Q.clear();
    H.clear();

    Vector v(size);
    Vector g(size);

    Vector c(maxiter + 1); 
    Vector s(maxiter + 1);
    Vector z(maxiter + 1);

    Vector h(size); // working column of H
   
    A.residuum(x, b, v);   //v = b - prod(A, x);

    B.apply(v, g);

    column(Q, 0) = g/ublas::norm_2(g);

    z(0) = ublas::norm_2(g);

    double z_0 = z(0);

    double alpha = 0;

    double t = tol + 1;

    unsigned int k = 0;


    h.clear();
    while (k < maxiter && t > tol){
    
        A.mult ( column(Q, k), v );  
        B.apply(v, g);

        for (unsigned int j = 0; j <= k; ++j) {
            ublas::matrix_column<Matrix> q = column(Q, j);
            h(j) = ublas::inner_prod(q, g);
            g -=  h(j)*q;
        }

        for (unsigned int i = 0; i < k; ++i) {
            double h_i   = c(i+1)*h(i) + s(i+1)*h(i+1);
            double h_i1  = s(i+1)*h(i) - c(i+1)*h(i+1);
            h(i)   = h_i;
            h(i+1) = h_i1;
        }

        h(k+1) = ublas::norm_2(g);
        column(Q, k+1) = g/h(k+1);

        double h_kk = h(k);
        double h_k1k = h(k+1);
        alpha  = sqrt(h_kk*h_kk + h_k1k*h_k1k);
        s(k+1) = h_k1k/alpha;
        c(k+1) = h_kk/alpha;
        h(k) = alpha;

        column(H, k) = h;

        z(k+1) = s(k+1)*z(k);
        z(k)   = c(k+1)*z(k);

        t=  z(k+1)/z_0;
        
        
        
        ++k;

        if (0) {
            Vector c(maxiter + 1); 
            c.clear();

            if (k>0) {
                unsigned int i = k;
                while ( (i--) > 0 ) {
                    c(i) = (z(i) - inner_prod(project(row(H, i), ublas::range(i+1, k+1) ), project( c, ublas::range(i+1, k+1) )))/H(i, i);
                }
            }

            Vector current_x(x);
            current_x += prod(project(Q, ublas::range::all(), ublas::range(0, k)), project(c, ublas::range(0, k) ) );

            std::cout << k << " " << current_x << "\n";
        }
    }

    c.clear();

    if (k>0) {
        unsigned int i = k;
        while ( (i--) > 0 ) {
            c(i) = (z(i) - inner_prod(project(row(H, i), ublas::range(i+1, k+1) ), project( c, ublas::range(i+1, k+1) )))/H(i, i);
        }
    }

    /*
      The above for-cycle can be replaced by a triangular solver. One
      needs to assure that the solver doesn't consider the
      underdiagonal elements of the Hessenberg matrix and works only
      with the first k rows and columns.  for (int i = 0; i < k; ++i)
      H(i+1, i) = 0;

      dusolve_gmres(H, c, z);
    */

    x += prod(project(Q, ublas::range::all(), ublas::range(0, k)), project(c, ublas::range(0, k) ) );
        
    return k;
    
}


/** \brief the gmres algorithm restarting after maxiter steps. nrestarts is the number of the allowed restarts
* - \param LinOp is the type of the linear opeartor representing the matrix
* - \param Vector is the type of the vectors
* - \param Matrix is the type of the matrix
* - \param Precond is the type of the used preconditioner
* - \note the input parameter x must be predefined
*/
template <class Matrix, class LinOp, class Vector, class Precond>
int gmres_restarts (LinOp const & A, 
                    Vector & x, 
                    const Vector & b, 
                    Precond const & B, 
                    unsigned int const maxiter, 
                    unsigned int const nrestarts, 
                    double const tol)
{
    unsigned int i = 0;
    unsigned int iter = 0;
    bool converged = false;

    Vector resid( b.size() );
    A.residuum(x,b,resid);

    double z_0 = norm_2(resid);

    while (i <= nrestarts && ! converged) { 
        unsigned int ret = gmres_step< Matrix > (A, x, b, B, maxiter, tol);
        A.residuum(x,b,resid);
        converged = ((norm_2(resid) / z_0) < tol);
        iter += ret;
        ++i;
    }

    return iter;
}

/** \brief /QGMRES. Only the last "number" vectors are taken into account, i.e. the orthogonalization depends only on the last number vectors.
* - The solution x is updated at the end of each iteration.
* - \param LinOp is the type of the linear opeartor representing the matrix
* - \param Vector is the type of the vectors
* - \param Matrix is the type of the matrix
* - \param Precond is the type of the used preconditioner
* - \note the input parameter x must be predefined
*/
template <class Matrix, class LinOp, class Vector, class Precond>
int gmres_short (LinOp const & A, 
                 Vector & x, 
                 const Vector & b, 
                 Precond const & B, 
                 int number, 
                 int maxiter, 
                 double tol)
{

  int size = A.image_size();

  Matrix Q(size, number+1);  Q.clear();
  Matrix P(size, number);    P.clear();

  Vector v(size);
  Vector v2(size);
  Vector g(size);
  Vector c(number+1);
  Vector s(number+1);

  Vector sum(size);
  Vector h(number + 1);

  A.residuum(x, b, v);
  B.apply(v, g);

  column(Q, 0) = g/ublas::norm_2(g);

  double z_0 = ublas::norm_2(g);
  double alpha = 0;
  double t = tol + 1;
  unsigned int k = 0;

  double z_1 = z_0;
  double z_2 = z_0;
    
  bool end = false;

  while (k < maxiter && !end){
        
    z_1 = z_2;
    z_2 = 0;

    A.mult ( column(Q, k % (number + 1)), v );
    B.apply(v, g);

    int lower = max<int>(0, k-number);

    for (int j = lower; j <= k; ++j) {
      ublas::matrix_column<Matrix> q = column(Q, j % (number+1));
      h(j % (number + 1)) = ublas::inner_prod(q, g);
      g -=  h(j % (number + 1))*q;
    }

    double h_i = 0;

    for (int i = lower; i < k; ++i) {
      double h_i  = c((i+1) % (number + 1))*h(i % (number + 1)) + s((i+1) % (number + 1))*h((i+1) % (number + 1));
      double h_i1 = s((i+1) % (number + 1))*h(i % (number + 1)) - c((i+1) % (number + 1))*h((i+1) % (number + 1));
      h(i % (number + 1))     = h_i;
      h((i+1) % (number + 1)) = h_i1;
    }

    double h_k1 = ublas::norm_2(g);
    column(Q, (k+1) % (number+1)) = g/h_k1;
        
    double h_kk = h(k % (number + 1));
        
    alpha  = sqrt(h_kk*h_kk + h_k1*h_k1);
    s((k+1) % (number + 1)) = h_k1/alpha;
    c((k+1) % (number + 1)) = h_kk/alpha;
    h(k % (number + 1)) = alpha;

       
    z_2 = s((k+1) % (number + 1))*z_1;
    z_1 = c((k+1) % (number + 1))*z_1;
        
    sum = h(lower % (number + 1))*column(P, lower % number);
    for (int i=lower+1; i < k; ++i){
      sum += h(i % (number + 1))*column(P, i%number);
    }

    column(P, k % number) = (column(Q, k % (number +1)) - sum)/h(k % (number + 1));

    x += z_1 * column(P, k % number);
        
    h((k+1) % (number + 1) ) = h_k1;

    t = sqrt(k+1) * (z_2/z_0);

    if (t < tol) {
      end = true;
    }
            
    ++k;

    // std::cout << k << " " << x << "\n";

  }
    
  return k;
}




#endif
