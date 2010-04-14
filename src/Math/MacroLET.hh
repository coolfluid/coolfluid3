#ifndef COOLFluiD_MathTools_MacroLET_hh
#define COOLFluiD_MathTools_MacroLET_hh

//////////////////////////////////////////////////////////////////////////////

#include "MathTools/VectorLET.hh"
#include "MathTools/MatrixLET.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {
  
  namespace MathTools {
    
//////////////////////////////////////////////////////////////////////////////

/** 
 * This file defines some macros useful for using the size deducing fast 
 * expression template techique.
 *
 * @author Andrea Lani
 *
 */

//////////////////////////////////////////////////////////////////////////////

/// High Performance vector with dynamically allocated memory requires the 
/// instantiation of each variable of its type on a 
/// different line
#define HPVECTORDYN  COOLFluiD::MathTools::VectorLET<CFreal, COOLFluiD::MathTools::Tag<SELF,__LINE__> >

/// High Performance vector with fixed size requires the 
/// instantiation of each variable of its type on a 
/// different line
#define HPVECTOR(nn)  COOLFluiD::MathTools::VectorLET<CFreal, COOLFluiD::MathTools::Tag<SELF,__LINE__>,nn>    

/// High Performance square matrix with dynamically allocated memory requires the 
/// instantiation of each variable of its type on a 
/// different line
#define HPMATRIXDYN   COOLFluiD::MathTools::MatrixLET<CFreal, COOLFluiD::MathTools::Tag<SELF,__LINE__> >

/// High Performance square matrix with fixed size requires the 
/// instantiation of each variable of its type on a 
/// different line
#define HPMATRIX(nn)  COOLFluiD::MathTools::MatrixLET<CFreal, COOLFluiD::MathTools::Tag<SELF,__LINE__>,nn>    

/// SCALAR requires the instantiation of each variable of its type on a 
/// different line
#define SCALAR  COOLFluiD::MathTools::Scalar<CFreal, COOLFluiD::MathTools::Tag<SELF,__LINE__> >

//////////////////////////////////////////////////////////////////////////////
    
   } // namespace MathTools

}   // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////
  
#endif 
