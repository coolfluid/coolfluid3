#define BOOST_DISABLE_ASSERTS

#include <boost/multi_array.hpp>
//#include <boost/multi_array/storage_order.hpp>
#include <boost/timer.hpp>
#include <boost/program_options.hpp>

#include <cstdio>
#include <iostream>

// #define STATIC_SIZES

int main(int argc, char * argv[])
{

#ifndef STATIC_SIZES

  // Declare the supported options.
  boost::program_options::options_description desc("allowed options");
  desc.add_options()
      ("help", "produce help message")
      ("nbrows", boost::program_options::value<int>(), "nb rows")
      ("nbcols", boost::program_options::value<int>(), "nb cols")
      ("iter",   boost::program_options::value<int>(), "nb iterations");

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count("help")) {
      std::cout << desc << "\n";
      return 1;
  }

  int nbrows;
  if ( vm.count("nbrows") ) {
    nbrows = vm["nbrows"].as<int>();
    std::cout << "NB ROWS " << nbrows << ".\n";
  } else {
    std::cout << "nbrows was not set.\n";
    return 1;
  }

  int nbcols;
  if ( vm.count("nbcols") ) {
    nbcols =  vm["nbcols"].as<int>();
    std::cout << "NB COLS " << nbcols << ".\n";
  } else {
    std::cout << "nbcols was not set.\n";
    return 1;
  }

  int iter = 1;
  if ( vm.count("iter") ) {
    iter = vm["iter"].as<int>();
    std::cout << "NB ITER " << iter << ".\n";
  }

#else

  const int nbrows = 500000;
  const int nbcols = 20;
  const int iter = 500;

#endif

  //------------------Measure boost creation ----------------------------------------------

  boost::timer boost_ctimer;

  // Create the boost array
  typedef boost::multi_array<double, 2> Array_t;
  Array_t boost_mat( boost::extents[nbrows][nbcols]  );
//  Array_t boost_mat( boost::extents[nbrows][nbcols] , boost::fortran_storage_order() );

  printf("[Boost]  Creation time: %6.3f seconds\n", boost_ctimer.elapsed() );

  //------------------Measure boost creation ----------------------------------------------

  boost::timer native_ctimer;

  // Create the native array
  double *native_mat = new double [nbrows * nbcols];

  printf("[Native] Creation time: %6.3f seconds\n", native_ctimer.elapsed() );

  //------------------Measure boost----------------------------------------------

  boost::timer boost_timer;

  int p1 = 0;
  for (int i = 0; i < iter; ++i)
  {
    for (int x = 0; x < nbrows; ++x)
    {
      for (int y = 0; y < nbcols; ++y)
      {
        boost_mat[x][y] = ++p1 ;
      }
    }
  }

  printf("[Boost]  Elapsed time: %6.3f seconds\n", boost_timer.elapsed() );

  double * bmat = boost_mat.data();

  for ( int i = 0; i < nbcols * nbrows ; ++i)
  {
    std::cout << bmat[i] << std::endl;
  }

  //------------------Measure boost----------------------------------------------


  boost::timer hboost_timer;

  int p2 = 0;
  for (int i = 0; i < iter; ++i)
  {
    for (int x = 0; x < nbrows; ++x)
    {
      for (int y = 0; y < nbcols; ++y)
      {
        bmat[y + (x * nbcols)] = ++p2 ;
      }
    }
  }

  printf("[BoostH]  Elapsed time: %6.3f seconds\n", hboost_timer.elapsed() );

    for ( int i = 0; i < nbcols * nbrows ; ++i)
    {
      std::cout << bmat[i] << std::endl;
    }


  //------------------Measure native-----------------------------------------------

  boost::timer native_timer;

  int p3 = 0;
  for (int i = 0; i < iter; ++i)
  {
    for (int x = 0; x < nbrows; ++x)
    {
      for (int y = 0; y < nbcols; ++y)
      {
        native_mat[x + (y * nbrows)] =  ++p3 ;
      }
    }
  }

  printf("[Native] Elapsed time: %6.3f seconds\n", native_timer.elapsed() );

  return 0;
}
