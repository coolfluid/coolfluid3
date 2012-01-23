#include <iostream>
#include <fstream>

#include <cstdio>
#include <cmath>

#include <boost/timer.hpp>
#include <boost/program_options.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "viennacl/scalar.hpp"
#include "viennacl/vector.hpp"
#include "viennacl/matrix.hpp"
#include "viennacl/compressed_matrix.hpp"
#include "viennacl/linalg/prod.hpp"  
#include "viennacl/linalg/compressed_matrix_operations.hpp"       //generic matrix-vector product
#include <omp.h>



int main(int argc, char * argv[])
{

  // create matrice & vectors -----------------------------------------------------

  int n_blocks = 1000;
  int w_sub_a = 8;
  int h_sub_a = 8;
  int w_A = 1 * w_sub_a;
  int h_A = 1 * h_sub_a;
  
  boost::numeric::ublas::vector<float>      b(w_A);
  boost::numeric::ublas::vector<float>      c(h_A);
  
  
  
  
  boost::numeric::ublas::compressed_matrix<float> A(h_A, w_A);
  viennacl::compressed_matrix<float>  vcl_A(h_A, w_A);
  
  // initialize matrice & vectors -----------------------------------------------------
  
  for( int i = 0; i < w_A; i++ )
  {
      b[i] = rand();
  }
//  for( int i = 0; i < n_blocks; i++ )
  {
  int i = 0;
     A(i*h_sub_a,i*h_sub_a) = rand();
     A(i*h_sub_a,i*h_sub_a+1) = rand();
     A(i*h_sub_a+1,i*h_sub_a) = rand();
     A(i*h_sub_a+1,i*h_sub_a+1) = rand();
     
  }
  
  //
  // Set up some ViennaCL objects
  //
  //viennacl::vector<float> vcl_b(static_cast<unsigned int>(b.size()));
  //viennacl::vector<float> vcl_c(static_cast<unsigned int>(c.size())); 
  viennacl::matrix<float> vcl_matrix(static_cast<unsigned int>(c.size()), static_cast<unsigned int>(b.size()));


  // run with native code ---------------------------------------------------


    boost::timer ntimer;
    
    for( int i = 0; i< n_blocks; i++ )
      c=boost::numeric::ublas::prod(A,b);

    std::cout<<"[native] time: " << ntimer.elapsed() <<" seconds\n";


  // run with opencl code -----------------------------------------------------
    
    copy(A, vcl_A);
    
    boost::timer Timer;
    
    # pragma omp parallel for \
	default ( shared )
    for( int i = 0; i < n_blocks; i++ )
    {  
        viennacl::vector<float> vcl_b(w_A);
        viennacl::vector<float> vcl_c(h_A); 
    
        copy(b, vcl_b);
        vcl_c=viennacl::linalg::prod(vcl_A,vcl_b);
        copy(vcl_c,c );
    }
    
    std::cout<<"[native] time: " << Timer.elapsed() <<" seconds\n";
 
  // clean up memory -------------------------------------------------------

  return 0;
}
