#include "JacobiEigenSolver.hpp"
#include "MathChecks.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

#define SMTX_MAX_ROTATIONS 20


////////////////////////////////////////////////////////////////////////////////
inline void JacobiEigenSolver::RotateMatrix(
    RealMatrix& a,
    Real& g, Real& h, Real& s, Real& c, Real& tau,
    const Uint& i, const Uint& j, const Uint& k, const Uint& l
    )
{
  g = a(i,j);
  h = a(k,l);
  a(i,j) = g - s*( h+g*tau );
  a(k,l) = h + s*( g-h*tau );
}
////////////////////////////////////////////////////////////////////////////////

void JacobiEigenSolver::eigenCalc( RealMatrix& a, RealMatrix& r, RealVector& lam)
{
  //CHECK( size == mNCols);

  cf_assert( a.nbRows() == a.nbCols() );

  Uint i, k, size = a.nbRows();
  Real tresh, theta, tau, t, sm, s, h, g, c, tmp;

  RealVector b(size);
  RealVector z(size);
  RealVector w(size);


  // initialize
  //mtxL.InitDelta( size);
  //mtxD.InitDelta( size);


  lam = RealVector( 0.0, size);


  r = RealMatrix( size, size, 0.0);
  for ( Uint ind=0; ind<size; ++ind)
    r( ind, ind) = 1.0;



  for ( Uint ip=0; ip<size; ++ip)
  {
    b[ip] = w[ip] = a(ip,ip);
    z[ip] = 0.0;
  }


  // begin rotation sequence
  for ( i=0; i<SMTX_MAX_ROTATIONS; ++i)
  {
    sm = 0.0;
    for ( Uint ip=0; ip<size-1; ++ip)
    {
      for ( Uint iq=ip+1; iq<size; ++iq)
      {
        sm += ::std::abs( a(ip,iq) );
      }
    }

    if ( sm == 0.0)
      break;

    if (i < 3)                                // first 3 sweeps
      tresh = 0.2*sm/(size*size);
    else
      tresh = 0.0;


    for ( int ip=0; ip< static_cast<int>(size-1); ++ip)
    {
      for ( int iq=ip+1; iq<static_cast<int>(size); ++iq)
      {
        g = 100.0 * ::std::abs( a(ip,iq) );

        // after 4 sweeps
        if ( i > 3 && ( ::std::abs( w[ip])+g) == ::std::abs( w[ip]) && (::std::abs( w[iq])+g) == ::std::abs( w[iq]))
        {
          a(ip,iq) = 0.0;
        }
        else if ( ::std::abs( a(ip,iq) ) > tresh)
        {
          h = w[iq] - w[ip];
          if ( ( ::std::abs(h)+g) == ::std::abs(h))
          {
            t = ( a(ip,iq) ) / h;
          }
          else
          {
            theta = 0.5*h / ( a(ip,iq) );
            t = 1.0 / ( ::std::abs(theta) + ::sqrt( 1.0+theta*theta) );
            if ( theta < 0.0)
            {
              t = -t;
            }
          }
          c = 1.0 / ::sqrt(1+t*t);
          s = t*c;
          tau = s/(1.0+c);
          h = t*a(ip,iq);
          z[ip] -= h;
          z[iq] += h;
          w[ip] -= h;
          w[iq] += h;
          a(ip,iq) = 0.0;

          // ip already shifted left by 1 unit
          for ( int j = 0; j <= static_cast<int>(ip-1); ++j)
          {
            RotateMatrix( a, g, h, s, c, tau, j, ip, j, iq );
          }

          // ip and iq already shifted left by 1 unit
          for ( int j = ip+1; j <= static_cast<int>(iq-1); ++j)
          {
            RotateMatrix( a, g, h, s, c, tau, ip, j, j, iq );
          }

          // iq already shifted left by 1 unit
          for ( int j=iq+1; j<static_cast<int>(size); ++j)
          {
            RotateMatrix( a, g, h, s, c, tau, ip, j, iq, j );
          }

          for ( int j=0; j<static_cast<int>(size); ++j)
          {
            RotateMatrix( r, g, h, s, c, tau, j, ip, j, iq );
          }

        }
      }
    }

    for ( Uint ip=0; ip<size; ++ip)
    {
      b[ip] += z[ip];
      w[ip] = b[ip];
      z[ip] = 0.0;
    }
  }


//   // this is NEVER called
//   if ( i >= SMTX_MAX_ROTATIONS )
//   {
//     std::cout << "SMatrix Decompose error: Error extracting eigenfunctions" << std::endl;
//   }


  // sort eigenfunctions                 these changes do not affect accuracy
  for ( Uint j=0; j<size-1; ++j)                  // boundary incorrect
  {
    k = j;
    tmp = w[k];
    for ( Uint i=j+1; i<size; ++i)                // boundary incorrect, shifted already
    {
      if ( w[i] > tmp)                   // why exchage if same?
      {
        k = i;
        tmp = w[k];
      }
    }
    if ( k != j)
    {
      w[k] = w[j];
      w[j] = tmp;
      for ( Uint i=0; i<size; i++)
      {
        tmp = r(i,j);
        r(i,j) = r(i,k);
        r(i,k) = tmp;
      }
    }
  }


  for ( Uint ip=0; ip<size; ++ip)
    lam[ip] = w[ip];

  return;


  // insure eigenvector consistency (i.e., Jacobi can compute vectors that
  // are negative of one another (.707,.707,0) and (-.707,-.707,0). This can
  // reek havoc in hyperstreamline/other stuff. We will select the most
  // positive eigenvector.
  int  ceil_half_n = (static_cast<int>(size) >> 1) + (static_cast<int>(size) & 1);
  int numPos;

  for ( int j=0; j<static_cast<int>(size); j++)
  {
    for ( numPos=0, i=0; i<size; ++i)
    {
      if ( r(i,j) >= 0.0 )
      {
        ++numPos;
      }
    }
  //    if ( numPos < ceil(double(n)/double(2.0)) )
    if ( numPos < ceil_half_n)
    {
      for( int i=0; i<static_cast<int>(size); ++i)
      {
        r(i,j) *= -1.0;
      }
    }
  }

  for ( Uint ip=0; ip<size; ++ip)
    lam[ip] = w[ip];

}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////
