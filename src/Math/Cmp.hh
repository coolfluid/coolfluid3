#ifndef COOLFluiD_MathTools_Cmp_hh
#define COOLFluiD_MathTools_Cmp_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/COOLFluiD.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

/// This struct allows us to determine at compile time
/// the bigger integer between N and M, with the compile time
/// constrain that if N != M one of the two has to be == 0
/// @author Andrea Lani
template <CFuint N, CFuint M>
struct CMP;

template <CFuint N>
struct CMP<N,0> {enum {MAX=N};};

template <CFuint M>
struct CMP<0,M> {enum {MAX=M};};

template <>
struct CMP<0,0> {enum {MAX=0};};

template <CFuint N>
struct CMP<N,N> {enum {MAX=N};};

#define NMAX(v1,v2) CMP<v1::SIZE,v2::SIZE>::MAX
#define NNMAX(n1,v2) CMP<n1,v2::SIZE>::MAX
#define GETSIZE(nn) ((nn > 0) ? nn : size())
#define GETMATSIZE(n1) ((n1 > 0)? n1*n1 : nbRows()*nbCols())
#define GETRCDIM(n1,n2) ((n1 > 0)? n1 : n2)

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

}   // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif
