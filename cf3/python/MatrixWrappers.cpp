// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "python/BoostPython.hpp"

#include "common/BasicExceptions.hpp"

#include "math/MatrixTypes.hpp"
#include "python/MatrixWrappers.hpp"

namespace cf3 {
namespace python {

using namespace boost::python;

#define TMATRIX Eigen::Matrix<Real,rows,cols>
#define TOTHERMATRIX Eigen::Matrix<Real,rows_other,cols_other>
#define TRETURNMATRIX Eigen::Matrix<Real,rows_return,cols_return>
#define TDYNMATRIX Eigen::Matrix<Real,-1, -1>
#define TVECTOR Eigen::Matrix<Real,rows,1>
#define TSQUAREMATRIX Eigen::Matrix<Real,rows,rows>
#define TDYNVECTOR Eigen::Matrix<Real,-1,1>
#define MIN(x,y) ((x)>(y)?(y):(x))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// init with size/value

// init a static vector or matrix, with 0
template<int rows, int cols>
boost::shared_ptr<TMATRIX > realmatrix_init_static_default(){
  boost::shared_ptr<TMATRIX >p(new TMATRIX(TMATRIX::Constant(0.0)));
  return p;
}

//init dynamic sized vector
template<int rows, int cols>
boost::shared_ptr<TMATRIX > realvector_init_size_default(const Uint size){
  boost::shared_ptr<TMATRIX >p(new TMATRIX(TMATRIX::Constant(size,0.0)));
  return p;
}

//init dynamic sized matrix
template<int rows, int cols>
boost::shared_ptr<TMATRIX > realmatrix_init_size_default(const Uint rownum, const Uint colnum){
  boost::shared_ptr<TMATRIX >p(new TMATRIX(TMATRIX::Constant(rownum,colnum,0.0)));
  return p;
}

//init static sized matrix/vector
template<int rows, int cols>
boost::shared_ptr<TMATRIX > realmatrix_init_static_value(const Real value){
  boost::shared_ptr<TMATRIX >p(new TMATRIX(TMATRIX::Constant(value)));
  return p;
}

//init dynamic sized vector
template<int rows, int cols>
boost::shared_ptr<TMATRIX > realvector_init_size_value(const Uint size, const Real value){
  boost::shared_ptr<TMATRIX >p(new TMATRIX(TMATRIX::Constant(size,value)));
  return p;
}

//init dynamic sized matrix
template<int rows, int cols>
boost::shared_ptr<TMATRIX > realmatrix_init_size_value(const Uint rownum, const Uint colnum, const Real value){
  boost::shared_ptr<TMATRIX >p(new TMATRIX(TMATRIX::Constant(rownum,colnum,value)));
  return p;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// init with copy

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > realmatrix_init_copy_dyn(const TMATRIX& other){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX(other));
  return p;
}

template<int rows, int cols, int rows_other, int cols_other>
boost::shared_ptr<TMATRIX > realmatrix_init_copy_static(const TOTHERMATRIX& other){
  boost::shared_ptr<TMATRIX >p(new TMATRIX(TMATRIX::Constant(0.0)));
  int rownum=MIN(rows,other.rows());
  int colnum=MIN(cols,other.cols());
  for (int i=0;i<rownum;i++){
    for (int j=0;j<colnum;j++){
      (*p)(i,j)=other(i,j);
    }
  }
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > realmatrix_init_copy_static_dyn(const TDYNMATRIX& other){
  boost::shared_ptr<TMATRIX >p(new TMATRIX(TMATRIX::Constant(0.0)));
  int rownum=MIN(rows,other.rows());
  int colnum=MIN(cols,other.cols());
  for (int i=0;i<rownum;i++){
    for (int j=0;j<colnum;j++){
      (*p)(i,j)=other(i,j);
    }
  }
  return p;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// init with table

template<int rows, int cols>
boost::shared_ptr<TMATRIX > realvector_init_tab(const list tab){
  Uint size=len(tab);
  TMATRIX *m=new TMATRIX(size);
  for (int i=0;i<size;i++){
    (*m)[i]=extract<Real>(tab[i]);
  }
  boost::shared_ptr<TMATRIX >p(m);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > realvector_init_static_tab(const list tab){
  TMATRIX *m=new TMATRIX(TMATRIX::Constant(0.0));
  Uint tab_size=len(tab);
  for (int i=0;i<tab_size;i++){
    (*m)[i]=extract<Real>(tab[i]);
  }
  boost::shared_ptr<TMATRIX >p(m);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > realmatrix_init_tab(const list tab){
  Uint rownum=len(tab);
  Uint colnum;
  TMATRIX *m;
  for (int i=0;i<rownum;i++){
    list temp=extract<list>(tab[i]);
    colnum=len(temp);
    if (i==0){
      m=new TMATRIX(rownum,colnum);
    }
    for (int j=0;j<colnum;j++){
      (*m)(i,j)=extract<Real>(temp[j]);
    }
  }
  boost::shared_ptr<TMATRIX >p(m);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > realmatrix_init_static_tab(const list tab){
  TMATRIX* m=new TMATRIX(TMATRIX::Constant(0.0));
  Uint rownum=len(tab);
  Uint colnum;
  for (int i=0;i<rownum;i++){
    list temp=extract<list>(tab[i]);
    colnum=len(temp);
    for (int j=0;j<colnum;j++){
      (*m)(i,j)=extract<Real>(temp[j]);
    }
  }
  boost::shared_ptr<TMATRIX >p(m);
  return p;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//static common
template<int rows, int cols>
void static_set_zero(boost::shared_ptr<TMATRIX > self){
  (*self).setZero();
}

template<int rows, int cols>
void static_set_ones(boost::shared_ptr<TMATRIX > self){
  (*self).setOnes();
}

template<int rows, int cols>
void static_set_constant(boost::shared_ptr<TMATRIX > self, Real value){
  (*self).setConstant(value);
}

template<int rows, int cols>
void static_set_random(boost::shared_ptr<TMATRIX > self){
  (*self).setRandom();
}

template<int rows, int cols>
void static_set_lin_spaced(boost::shared_ptr<TMATRIX > self, Real low, Real high){
  (*self).setLinSpaced(rows,low,high);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//dynamic  matrix
template<int rows, int cols>
void dynamic_matrix_set_zero(boost::shared_ptr<TMATRIX > self, Uint rownum, Uint colnum){
  (*self).setZero(rownum,colnum);
}

template<int rows, int cols>
void dynamic_matrix_set_ones(boost::shared_ptr<TMATRIX > self, Uint rownum, Uint colnum){
  (*self).setOnes(rownum,colnum);
}

template<int rows, int cols>
void dynamic_matrix_set_constant(boost::shared_ptr<TMATRIX > self, Uint rownum, Uint colnum, Real value){
  (*self).setConstant(rownum,colnum,value);
}

template<int rows, int cols>
void dynamic_matrix_set_random(boost::shared_ptr<TMATRIX > self, Uint rownum, Uint colnum){
  (*self).setRandom(rownum,colnum);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//dynamic vector
template<int rows, int cols>
void dynamic_vector_set_zero(boost::shared_ptr<TMATRIX > self, Uint size){
  (*self).setZero(size);
}

template<int rows, int cols>
void dynamic_vector_set_ones(boost::shared_ptr<TMATRIX > self, Uint size){
  (*self).setOnes(size);
}

template<int rows, int cols>
void dynamic_vector_set_constant(boost::shared_ptr<TMATRIX > self, Uint size, Real value){
  (*self).setConstant(size,value);
}

template<int rows, int cols>
void dynamic_vector_set_random(boost::shared_ptr<TMATRIX > self, Uint size){
  (*self).setRandom(size);
}

template<int rows, int cols>
void dynamic_vector_set_lin_spaced(boost::shared_ptr<TMATRIX > self, Uint size, Real low, Real high){
  (*self).setLinSpaced(size,low,high);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//dynamic  matrix default
template<int rows, int cols>
void dynamic_matrix_set_zero_default(boost::shared_ptr<TMATRIX > self){
  (*self).setZero((*self).rows(),(*self).cols());
}

template<int rows, int cols>
void dynamic_matrix_set_ones_default(boost::shared_ptr<TMATRIX > self){
  (*self).setOnes((*self).rows(),(*self).cols());
}

template<int rows, int cols>
void dynamic_matrix_set_constant_default(boost::shared_ptr<TMATRIX > self, Real value){
  (*self).setConstant((*self).rows(),(*self).cols(),value);
}

template<int rows, int cols>
void dynamic_matrix_set_random_default(boost::shared_ptr<TMATRIX > self){
  (*self).setRandom((*self).rows(),(*self).cols());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//dynamic vector default
template<int rows, int cols>
void dynamic_vector_set_zero_default(boost::shared_ptr<TMATRIX > self){
  (*self).setZero((*self).size());
}

template<int rows, int cols>
void dynamic_vector_set_ones_default(boost::shared_ptr<TMATRIX > self){
  (*self).setOnes((*self).size());
}

template<int rows, int cols>
void dynamic_vector_set_constant_default(boost::shared_ptr<TMATRIX > self, Real value){
  (*self).setConstant((*self).size(),value);
}

template<int rows, int cols>
void dynamic_vector_set_random_default(boost::shared_ptr<TMATRIX > self){
  (*self).setRandom((*self).size());
}

template<int rows, int cols>
void dynamic_vector_set_lin_spaced_default(boost::shared_ptr<TMATRIX > self, Real low, Real high){
  (*self).setLinSpaced((*self).size(),low,high);
}
/*template<int rows, int cols>
void dynamic_vector_unit(boost::shared_ptr<TMATRIX > self, Uint size, Uint i){
  self.Unit(size, i);
}*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Matrix op
template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_scalar_add(boost::shared_ptr<TMATRIX > self,const Real value){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self).array()+value));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_scalar_iadd(boost::shared_ptr<TMATRIX > self,const Real value){
  (*self)=(*self).array()+value;
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_scalar_sub(boost::shared_ptr<TMATRIX > self,const Real value){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self).array()-value));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_scalar_isub(boost::shared_ptr<TMATRIX > self,const Real value){
  (*self)=(*self).array()-value;
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_scalar_mul(boost::shared_ptr<TMATRIX > self,const Real value){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self)*value));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_scalar_imul(boost::shared_ptr<TMATRIX > self,const Real value){
  (*self)*=value;
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_scalar_div(boost::shared_ptr<TMATRIX > self,const Real value){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self)/value));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_scalar_idiv(boost::shared_ptr<TMATRIX > self,const Real value){
  (*self)/=value;
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_add(boost::shared_ptr<TMATRIX > self,const TMATRIX& other){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self)+other));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_iadd(boost::shared_ptr<TMATRIX > self,const TMATRIX& other){
  (*self)+=other;
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_sub(boost::shared_ptr<TMATRIX > self,const TMATRIX& other){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self)-other));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_isub(boost::shared_ptr<TMATRIX > self,const TMATRIX& other){
  (*self)-=other;
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_mul(boost::shared_ptr<TMATRIX > self,const TMATRIX& other){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self).array()*other.array()));
  return p;
}

template<int rows, int cols, int rows_other, int cols_other, int rows_return, int cols_return>
boost::shared_ptr<TRETURNMATRIX > matrix_by_matrix_left(boost::shared_ptr<TMATRIX > self,const TOTHERMATRIX& other){
  boost::shared_ptr<TRETURNMATRIX >p(new TRETURNMATRIX(other*(*self)));
  return p;
}

template<int rows, int cols, int rows_other, int cols_other, int rows_return, int cols_return>
boost::shared_ptr<TRETURNMATRIX > matrix_by_matrix_right(boost::shared_ptr<TMATRIX > self,const TOTHERMATRIX& other){
  boost::shared_ptr<TRETURNMATRIX >p(new TRETURNMATRIX((*self)*other));
  return p;
}

template<int rows, int cols, int rows_other, int cols_other>
boost::shared_ptr<TDYNMATRIX > matrix_by_matrix_dyn(boost::shared_ptr<TMATRIX > self,const TOTHERMATRIX& other){
  if (other.rows() == 1){
    boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX(other*(*self)));
    return p;
  }
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self)*other));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_imul(boost::shared_ptr<TMATRIX > self,const TMATRIX& other){
  (*self)=(*self).array()*other.array();
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_div(boost::shared_ptr<TMATRIX > self,const TMATRIX& other){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self).array()/other.array()));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_idiv(boost::shared_ptr<TMATRIX > self,const TMATRIX& other){
  (*self)=(*self).array()/other.array();
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//vector common
template<int rows, int cols>
Real vector_get_item(boost::shared_ptr<TMATRIX > self, const Uint i){
  return (*self)[i];
}

template<int rows, int cols>
void vector_set_item(boost::shared_ptr<TMATRIX > self, const Uint i, const Real value){
  (*self)[i] = value;
}

template<int rows, int cols>
Uint vector_size(boost::shared_ptr<TMATRIX > self){
  return (*self).size();
}

template<int rows, int cols>
std::string vector_str(boost::shared_ptr<TMATRIX > self){
  std::ostringstream s;
  int colnum=(*self).size();
  s << (*self)[0];
  for (int i=1;i<colnum;i++)
    s << "\n" << (*self)[i];
  s << "\n";
  return s.str();
}

template<int rows, int cols>
boost::shared_ptr<TDYNVECTOR > vector_head(boost::shared_ptr<TVECTOR > self, Uint size){
  boost::shared_ptr<TDYNVECTOR >p(new TDYNVECTOR((*self).head(size)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNVECTOR > vector_tail(boost::shared_ptr<TVECTOR > self, Uint size){
  boost::shared_ptr<TDYNVECTOR >p(new TDYNVECTOR((*self).tail(size)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNVECTOR > vector_segment(boost::shared_ptr<TVECTOR > self, Uint ind, Uint size){
  boost::shared_ptr<TDYNVECTOR >p(new TDYNVECTOR((*self).segment(ind,size)));
  return p;
}

template<int rows, int cols>
void vector_resize(boost::shared_ptr<TMATRIX > self, const Uint size){
  (*self).resize(size);
}

template<int rows, int cols>
Real vector_dot(boost::shared_ptr<TMATRIX > self, const TMATRIX& other){
  return (*self).dot(other);
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > vector_cross(boost::shared_ptr<TMATRIX > self, const TMATRIX& other){
  return boost::shared_ptr<TMATRIX >(new TMATRIX((*self).cross(other)));
}

/*template<int rows, int cols>
boost::shared_ptr<TMATRIX > vector_cross(boost::shared_ptr<TMATRIX > self, const TMATRIX& other){
  return boost::shared_ptr<TMATRIX >(new TMATRIX((*self).cross(other)));
}*/

template<int rows, int cols>
void matrix_resize(boost::shared_ptr<TMATRIX > self, const Uint rownum, const Uint colnum){
  (*self).resize(rownum,colnum);
}

template<int rows, int cols>
Real vector_norm(boost::shared_ptr<TMATRIX > self){
  return (*self).norm();
}

template<int rows, int cols>
Real vector_squared_norm(boost::shared_ptr<TMATRIX > self){
  return (*self).squaredNorm();
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > vector_normalized(boost::shared_ptr<TMATRIX > self){
  return boost::shared_ptr<TMATRIX >(new TMATRIX((*self).normalized()));
}

template<int rows, int cols>
void vector_normalize(boost::shared_ptr<TMATRIX > self){
  (*self).normalize();
}

template<int rows, int cols>
boost::shared_ptr<TSQUAREMATRIX > vector_as_diagonal(boost::shared_ptr<TMATRIX > self){
  boost::shared_ptr<TSQUAREMATRIX >p(new TSQUAREMATRIX((*self).asDiagonal()));
  return p;
}

template<int rows, int cols>
void matrix_resize_like(boost::shared_ptr<TMATRIX > self,const TMATRIX& other){
  (*self).resizeLike(other);
}

template<int rows, int cols>
void vector_conservative_resize(boost::shared_ptr<TMATRIX > self, const Uint size){
  (*self).conservativeResize(size);
}

template<int rows, int cols>
void matrix_conservative_resize(boost::shared_ptr<TMATRIX > self, const Uint rownum, const Uint colnum){
  (*self).conservativeResize(rownum,colnum);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//matrix common
template<int rows, int cols>
Real matrix_get_item(boost::shared_ptr<TMATRIX > self, const tuple &t){
  return (*self)(extract<Uint>(t[0]),extract<Uint>(t[1]));
}

template<int rows, int cols>
void matrix_set_item(boost::shared_ptr<TMATRIX > self, const tuple &t, const Real value){
  (*self)(extract<Uint>(t[0]),extract<Uint>(t[1])) = value;
}

template<int rows, int cols>
Uint matrix_size(boost::shared_ptr<TMATRIX > self){
  return (*self).size();
}

template<int rows, int cols>
std::string matrix_str(boost::shared_ptr<TMATRIX > self){
  std::ostringstream s;
  int rownum=rows>0?rows:(*self).rows();
  int colnum=cols>0?cols:(*self).cols();
  for (int i=0;i<rownum;i++){
    for (int j=0;j<colnum;j++){
      s << (*self)(i,j) << " ";
    }
    s << "\n";
  }
  return s.str();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<int rows, int cols>
Uint matrix_rows(boost::shared_ptr<TMATRIX > self){
  return (*self).rows();
}

template<int rows, int cols>
Uint matrix_cols(boost::shared_ptr<TMATRIX > self){
  return (*self).cols();
}

template<int rows, int cols>
Uint matrix_inner_size(boost::shared_ptr<TMATRIX > self){
  return (*self).innerSize();
}

template<int rows, int cols>
Uint matrix_outer_size(boost::shared_ptr<TMATRIX > self){
  return (*self).outerSize();
}
template<int rows, int cols>
Uint matrix_inner_stride(boost::shared_ptr<TMATRIX > self){
  return (*self).innerStride();
}

template<int rows, int cols>
Uint matrix_outer_stride(boost::shared_ptr<TMATRIX > self){
  return (*self).outerStride();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_get_row(boost::shared_ptr<TMATRIX > self, Uint row){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).row(row)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_get_col(boost::shared_ptr<TMATRIX > self, Uint col){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).col(col)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TVECTOR > matrix_get_row_vector(boost::shared_ptr<TMATRIX > self, Uint row){
  boost::shared_ptr<TVECTOR >p(new TVECTOR((*self).row(row)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TVECTOR > matrix_get_col_vector(boost::shared_ptr<TMATRIX > self, Uint col){
  boost::shared_ptr<TVECTOR >p(new TVECTOR((*self).col(col)));
  return p;
}

template<int rows, int cols>
void matrix_set_row(boost::shared_ptr<TMATRIX > self, Uint row, const TDYNMATRIX& value){
  Uint size=(*self).cols();
  if (value.cols() > value.rows()){
    for (int i=0;i<size;i++)
      (*self)(row,i)=value(0,i);
  }else{
    for (int i=0;i<size;i++)
      (*self)(row,i)=value(i,0);
  }
}

template<int rows, int cols>
void matrix_set_col(boost::shared_ptr<TMATRIX > self, Uint col, const TDYNMATRIX& value){
  Uint size=(*self).rows();
  if (value.cols() > value.rows()){
    for (int i=0;i<size;i++)
      (*self)(i,col)=value(0,i);
  }else{
    for (int i=0;i<size;i++)
      (*self)(i,col)=value(i,0);
  }
}

template<int rows, int cols>
void matrix_set_row_vector(boost::shared_ptr<TMATRIX > self, Uint row, const TVECTOR& value){
  int colnum=cols>0?cols:(*self).cols();
  for (int i=0;i<colnum;i++)
    (*self)(row,i)=value[i];
}

template<int rows, int cols>
void matrix_set_col_vector(boost::shared_ptr<TMATRIX > self, Uint col, const TVECTOR& value){
  int rownum=rows>0?rows:(*self).rows();
  for (int i=0;i<rownum;i++)
    (*self)(i,col)=value[i];
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_block(boost::shared_ptr<TMATRIX > self, Uint col, Uint row, Uint colnum, Uint rownum){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).block(col,row,colnum,rownum)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_top_left_corner(boost::shared_ptr<TMATRIX > self, Uint colnum, Uint rownum){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).topLeftCorner(colnum,rownum)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_top_right_corner(boost::shared_ptr<TMATRIX > self, Uint colnum, Uint rownum){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).topRightCorner(colnum,rownum)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_bottom_left_corner(boost::shared_ptr<TMATRIX > self, Uint colnum, Uint rownum){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).bottomLeftCorner(colnum,rownum)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_bottom_right_corner(boost::shared_ptr<TMATRIX > self, Uint colnum, Uint rownum){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).bottomRightCorner(colnum,rownum)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_top_rows(boost::shared_ptr<TMATRIX > self, Uint rownum){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).topRows(rownum)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_bottom_rows(boost::shared_ptr<TMATRIX > self, Uint rownum){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).bottomRows(rownum)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_left_cols(boost::shared_ptr<TMATRIX > self, Uint colnum){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).leftCols(colnum)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TDYNMATRIX > matrix_right_cols(boost::shared_ptr<TMATRIX > self, Uint colnum){
  boost::shared_ptr<TDYNMATRIX >p(new TDYNMATRIX((*self).rightCols(colnum)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_adjoint(boost::shared_ptr<TMATRIX > self){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self).adjoint()));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_transpose(boost::shared_ptr<TMATRIX > self){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self).transpose()));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_iadjoint(boost::shared_ptr<TMATRIX > self){
  (*self).adjointInPlace();
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_itranspose(boost::shared_ptr<TMATRIX > self){
  (*self).transposeInPlace();
  boost::shared_ptr<TMATRIX >p(self);
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_inverse(boost::shared_ptr<TMATRIX > self){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self).inverse()));
  return p;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<int rows, int cols>
Real matrix_max(boost::shared_ptr<TMATRIX > self){
  return (*self).maxCoeff();
}

template<int rows, int cols>
Real matrix_mean(boost::shared_ptr<TMATRIX > self){
  return (*self).sum()/((Real)(*self).size());
}

template<int rows, int cols>
Real matrix_min(boost::shared_ptr<TMATRIX > self){
  return (*self).minCoeff();
}

template<int rows, int cols>
Real matrix_norm(boost::shared_ptr<TMATRIX > self){
  return (*self).norm();
}

template<int rows, int cols>
Real matrix_squared_norm(boost::shared_ptr<TMATRIX > self){
  return (*self).squaredNorm();
}

template<int rows, int cols>
boost::shared_ptr<TMATRIX > matrix_normalized(boost::shared_ptr<TMATRIX > self){
  boost::shared_ptr<TMATRIX >p(new TMATRIX((*self).normalized()));
  return p;
}

template<int rows, int cols>
void matrix_normalize(boost::shared_ptr<TMATRIX > self){
  (*self).normalize();
}

template<int rows, int cols>
void matrix_identity_static(boost::shared_ptr<TMATRIX > self){
  (*self)=TMATRIX::Identity();
}

template<int rows, int cols>
void matrix_identity_dynamic(boost::shared_ptr<TMATRIX > self){
  (*self)=TMATRIX::Identity((*self).rows(),(*self).cols());
}

template<int rows, int cols>
boost::shared_ptr<TVECTOR > matrix_get_diagonal_ind(boost::shared_ptr<TMATRIX > self,const int n){
  boost::shared_ptr<TVECTOR >p(new TVECTOR((*self).diagonal(n)));
  return p;
}

template<int rows, int cols>
boost::shared_ptr<TVECTOR > matrix_get_diagonal_default(boost::shared_ptr<TMATRIX > self){
  return matrix_get_diagonal_ind(self,0);
}

template<int rows, int cols, int rows_other, int cols_other>
void matrix_set_diagonal_ind(boost::shared_ptr<TMATRIX > self,const int n,const TOTHERMATRIX& other){
  (*self).diagonal(n)=other;
}

template<int rows, int cols, int rows_other, int cols_other>
void matrix_set_diagonal_default(boost::shared_ptr<TMATRIX > self,const TOTHERMATRIX& other){
  matrix_set_diagonal_ind(self,0,other);
}

template<int rows, int cols>
Real matrix_determinant(boost::shared_ptr<TMATRIX > self){
  return (*self).determinant();
}

template<int rows, int cols>
boost::shared_ptr<TVECTOR > matrix_eigenvalues(boost::shared_ptr<TMATRIX > self){
  boost::shared_ptr<TVECTOR >p(new TVECTOR((*self).eigenvalues()));
  return p;
}

template<int rows, int cols>
Real matrix_trace(boost::shared_ptr<TMATRIX > self){
  return (*self).trace();
}

template<int rows, int cols>
void matrix_diagonal(boost::shared_ptr<TMATRIX > self, const Real value){
  (*self).diagonal()=TVECTOR::Constant(value);
}

template<int rows, int cols>
bool matrix_is_constant(boost::shared_ptr<TMATRIX > self, const Real value , const Real prec){
  return (*self).isConstant(value,prec);
}

template<int rows, int cols>
bool matrix_is_diagonal(boost::shared_ptr<TMATRIX > self, const Real prec){
  return (*self).isDiagonal(prec);
}

template<int rows, int cols>
bool matrix_is_identity(boost::shared_ptr<TMATRIX > self, const Real prec){
  return (*self).isIdentity(prec);
}

template<int rows, int cols>
bool matrix_is_approx(boost::shared_ptr<TMATRIX > self, boost::shared_ptr<TMATRIX > other, const Real prec){
  return (*self).isApprox(*other,prec);
}

template<int rows, int cols>
bool matrix_is_equal(boost::shared_ptr<TMATRIX > self, boost::shared_ptr<TMATRIX > other){
  return (*self).isApprox(*other,0.0001);
}

template<int rows, int cols>
bool matrix_is_zero(boost::shared_ptr<TMATRIX > self , const Real prec){
  return (*self).isZero(prec);
}

/** @brief these definitions are only applied on a vector
  */
template<int rows, int cols>
void def_common_vector(class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >*matrix_class){
  matrix_class->def("__getitem__", vector_get_item<rows,cols>);
  matrix_class->def("__setitem__", vector_set_item<rows,cols>);
  matrix_class->def("__len__", vector_size<rows,cols>);
  matrix_class->def("__str__", vector_str<rows,cols>);
  matrix_class->def("head", vector_head<rows,cols>);
  matrix_class->def("tail", vector_tail<rows,cols>);
  matrix_class->def("segment", vector_segment<rows,cols>);
  matrix_class->def("setZero", dynamic_vector_set_zero<rows,cols>);
  matrix_class->def("setOnes", dynamic_vector_set_ones<rows,cols>);
  matrix_class->def("setConstant", dynamic_vector_set_constant<rows,cols>);
  matrix_class->def("setRandom", dynamic_vector_set_random<rows,cols>);
  matrix_class->def("setLinSpaced", dynamic_vector_set_lin_spaced<rows,cols>);
  matrix_class->def("innerStride", matrix_inner_stride<rows,cols>);
  matrix_class->def("asDiagonal", vector_as_diagonal<rows,cols>);
  //matrix_class->def("mul", vector_by_matrix<rows,cols>);
}

/** @brief these definitions are only applied on a static matrixs and static vectors
  */
template<int rows, int cols>
void def_common_static(class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >*matrix_class){
  matrix_class->def("__init__", make_constructor(&realmatrix_init_static_default<rows,cols>));
  matrix_class->def("__init__", make_constructor(&realmatrix_init_static_value<rows,cols>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_static<rows,cols,rows,cols>));
  matrix_class->def("setZero", static_set_zero<rows,cols>);
  matrix_class->def("setOnes", static_set_ones<rows,cols>);
  matrix_class->def("setConstant", static_set_constant<rows,cols>);
  matrix_class->def("setRandom", static_set_random<rows,cols>);
  matrix_class->def("max",matrix_max<rows,cols>);
  matrix_class->def("mean",matrix_mean<rows,cols>);
  matrix_class->def("min",matrix_min<rows,cols>);
  //matrix_class->def("setLinSpaced", static_set_lin_spaced<rows,cols>);
}

/** @brief these definitions are only applied on dynamic matrixs and dynamic vectors
  */
template<int rows, int cols>
void def_common_dynamic(class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >*matrix_class){
  matrix_class->def("__init__", make_constructor(&realmatrix_init_copy_dyn<rows,cols>));
}

/** @brief these definitions are only applied on a matrix
  */
template<int rows, int cols>
void def_common_pure_matrix(class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >*matrix_class){
  matrix_class->def("__getitem__", matrix_get_item<rows,cols>);
  matrix_class->def("__setitem__", matrix_set_item<rows,cols>);
  matrix_class->def("__len__", matrix_size<rows,cols>);
  matrix_class->def("__str__", matrix_str<rows,cols>);
  matrix_class->def("col", matrix_get_col<rows,cols>, "col(col_number) -> RealMatrix(n,1)");
  matrix_class->def("row", matrix_get_row<rows,cols>, "row(row_number) -> RealMatrix(1,n)");
  matrix_class->def("colVector", matrix_get_col_vector<rows,cols>, "colVector(col_number) -> RealVector(n)");
  matrix_class->def("rowVector", matrix_get_row_vector<rows,cols>, "rowVector(row_number) -> RealVector(n)");
  matrix_class->def("setCol", matrix_set_col<rows,cols>, "setCol(col_number, RealMatrix(n,1)|RealMatrix(1,n)|RealVector(n))");
  matrix_class->def("setRow", matrix_set_row<rows,cols>, "setRow(row_number, RealMatrix(n,1)|RealMatrix(1,n)|RealVector(n))");
  matrix_class->def("setCol", matrix_set_col_vector<rows,cols>, "setCol(col_number, RealMatrix(n,1)|RealMatrix(1,n)|RealVector(n))");
  matrix_class->def("setRow", matrix_set_row_vector<rows,cols>, "setRow(row_number, RealMatrix(n,1)|RealMatrix(1,n)|RealVector(n))");
  matrix_class->def("block", matrix_block<rows,cols>, "block(row, col, row_number,col_number) -> RealMatrix(row_number,col_number)");
  matrix_class->def("topLeftCorner", matrix_top_left_corner<rows,cols>, "topLeftCorner(row_number,col_number) -> RealMatrix(row_number,col_number)");
  matrix_class->def("topRightCorner", matrix_top_right_corner<rows,cols>, "topLeftRight(row_number,col_number) -> RealMatrix(row_number,col_number)");
  matrix_class->def("bottomLeftCorner", matrix_bottom_left_corner<rows,cols>, "bottomLeftCorner(row_number,col_number) -> RealMatrix(row_number,col_number)");
  matrix_class->def("bottomRightCorner", matrix_bottom_right_corner<rows,cols>, "bottomRightCorner(row_number,col_number) -> RealMatrix(row_number,col_number)");
  matrix_class->def("topRows", matrix_top_rows<rows,cols>, "topRows(row_number) -> RealMatrix(n,row_number)");
  matrix_class->def("bottomRows", matrix_bottom_rows<rows,cols>, "bottomRows(row_number) -> RealMatrix(n,row_number)");
  matrix_class->def("leftCols", matrix_left_cols<rows,cols>, "leftCols(col_number) -> RealMatrix(col_number,n)");
  matrix_class->def("rightCols", matrix_right_cols<rows,cols>, "rightCols(col_number) -> RealMatrix(col_number,n)");
  matrix_class->def("setZero", dynamic_matrix_set_zero<rows,cols>);
  matrix_class->def("setOnes", dynamic_matrix_set_ones<rows,cols>);
  matrix_class->def("setConstant", dynamic_matrix_set_constant<rows,cols>);
  matrix_class->def("setRandom", dynamic_matrix_set_random<rows,cols>);
  matrix_class->def("rows", matrix_rows<rows,cols>);
  matrix_class->def("cols", matrix_cols<rows,cols>);
  matrix_class->def("innerSize", matrix_inner_size<rows,cols>);
  matrix_class->def("outerSize", matrix_outer_size<rows,cols>);
  matrix_class->def("innerStride", matrix_inner_stride<rows,cols>);
  matrix_class->def("outerStride", matrix_outer_stride<rows,cols>);
  matrix_class->def("adjoint", matrix_adjoint<rows,cols>);
  matrix_class->def("transposed", matrix_transpose<rows,cols>);
  matrix_class->def("adjointInPlace", matrix_iadjoint<rows,cols>);
  matrix_class->def("transpose", matrix_itranspose<rows,cols>);
  matrix_class->def("getDiagonal", matrix_get_diagonal_default<rows,cols>);
  matrix_class->def("setDiagonal", matrix_set_diagonal_default<rows,cols,rows,1>);
  matrix_class->def("getDiagonal", matrix_get_diagonal_ind<rows,cols>);
  matrix_class->def("setDiagonal", matrix_set_diagonal_ind<rows,cols,rows,1>);
  matrix_class->def("determinant", matrix_determinant<rows,cols>);
  matrix_class->def("trace", matrix_trace<rows,cols>);
  matrix_class->def("inverse", matrix_inverse<rows,cols>);
  matrix_class->def("isDiagonal",matrix_is_diagonal<rows,cols>);
  matrix_class->def("isIdentity",matrix_is_identity<rows,cols>);
}

/** @brief these definitions are applied to all types
  */
template<int rows, int cols>
void def_common_matrix(class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >*matrix_class){
  matrix_class->def("__add__", matrix_add<rows,cols>);
  matrix_class->def("__sub__", matrix_sub<rows,cols>);
  matrix_class->def("__iadd__", matrix_iadd<rows,cols>);
  matrix_class->def("__isub__", matrix_isub<rows,cols>);
  matrix_class->def("__mul__", matrix_mul<rows,cols>);
  matrix_class->def("__div__", matrix_div<rows,cols>);
  matrix_class->def("__imul__", matrix_imul<rows,cols>);
  matrix_class->def("__idiv__", matrix_idiv<rows,cols>);
  matrix_class->def("__add__", matrix_scalar_add<rows,cols>);
  matrix_class->def("__sub__", matrix_scalar_sub<rows,cols>);
  matrix_class->def("__mul__",matrix_scalar_mul<rows,cols>);
  matrix_class->def("__div__",matrix_scalar_div<rows,cols>);
  matrix_class->def("__iadd__", matrix_scalar_iadd<rows,cols>);
  matrix_class->def("__isub__", matrix_scalar_isub<rows,cols>);
  matrix_class->def("__imul__",matrix_scalar_imul<rows,cols>);
  matrix_class->def("__idiv__",matrix_scalar_idiv<rows,cols>);
  matrix_class->def("resizeLike", matrix_resize_like<rows,cols>);
  matrix_class->def("norm", vector_norm<rows,cols>);
  matrix_class->def("squaredNorm", vector_squared_norm<rows,cols>);
  matrix_class->def("normalized", vector_normalized<rows,cols>);
  matrix_class->def("normalize", vector_normalize<rows,cols>);
  matrix_class->def("__eq__",matrix_is_equal<rows,cols>);
  matrix_class->def("isZero",matrix_is_zero<rows,cols>);
  matrix_class->def("isConstant",matrix_is_constant<rows,cols>);
  matrix_class->def("isApprox",matrix_is_approx<rows,cols>);
}

/** @brief wrap the Eigen::Matrix<n,1>
  */
template<int rows, int cols>
void def_static_vector(const char* name,const char* doc){
  class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >*matrix_class = new class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >(name,doc);
  matrix_class->def("__init__",make_constructor(&realvector_init_static_tab<rows,cols>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_static<rows,cols,Eigen::Dynamic,cols>));
  matrix_class->def("mul", matrix_by_matrix_dyn<rows,cols,Eigen::Dynamic,Eigen::Dynamic>);
  matrix_class->def("mul", matrix_by_matrix_left<rows,cols,rows,rows,rows,cols>);
  matrix_class->def("setLinSpaced", static_set_lin_spaced<rows,cols>);
  def_common_vector<rows,cols>(matrix_class);
  def_common_matrix<rows,cols>(matrix_class);
  def_common_static<rows,cols>(matrix_class);
}

/** @brief wrap the Eigen::Matrix<Eigen::Dynamic,1>
  */
template<int rows, int cols>
void def_dynamic_vector(const char* name,const char* doc){
  class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >*matrix_class = new class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >(name,doc);
  matrix_class->def("__init__",make_constructor(&realvector_init_size_default<rows,cols>));
  matrix_class->def("__init__",make_constructor(&realvector_init_size_value<rows,cols>));
  matrix_class->def("__init__",make_constructor(&realvector_init_tab<rows,cols>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_dyn<4,1>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_dyn<3,1>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_dyn<2,1>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_dyn<1,1>));
  matrix_class->def("setZero", dynamic_vector_set_zero_default<rows,cols>);
  matrix_class->def("setOnes", dynamic_vector_set_ones_default<rows,cols>);
  matrix_class->def("setConstant", dynamic_vector_set_constant_default<rows,cols>);
  matrix_class->def("setRandom", dynamic_vector_set_random_default<rows,cols>);
  matrix_class->def("setLinSpaced", dynamic_vector_set_lin_spaced_default<rows,cols>);
  matrix_class->def("resize", vector_resize<rows,cols>);
  matrix_class->def("conservativeResize", vector_conservative_resize<rows,cols>);
  matrix_class->def("mul", matrix_by_matrix_left<rows,cols,rows,rows,rows,1>);
  matrix_class->def("mul", matrix_by_matrix_left<rows,cols,4,4,4,1>);
  matrix_class->def("mul", matrix_by_matrix_left<rows,cols,3,3,3,1>);
  matrix_class->def("mul", matrix_by_matrix_left<rows,cols,2,2,2,1>);
  def_common_vector<rows,cols>(matrix_class);
  def_common_matrix<rows,cols>(matrix_class);
  def_common_dynamic<rows,cols>(matrix_class);
}

/** @brief wrap the Eigen::Matrix<n,n>
  */
template<int rows, int cols>
void def_static_matrix(const char* name,const char* doc){
  class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >*matrix_class = new class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >(name,doc);
  matrix_class->def("__init__",make_constructor(&realmatrix_init_static_tab<rows,cols>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_static<rows,cols,Eigen::Dynamic,Eigen::Dynamic>));
  matrix_class->def("setDiagonal", matrix_set_diagonal_default<rows,cols,Eigen::Dynamic,1>);
  matrix_class->def("setDiagonal", matrix_set_diagonal_ind<rows,cols,Eigen::Dynamic,1>);
  matrix_class->def("identity", matrix_identity_static<rows,cols>);
  matrix_class->def("mul", matrix_by_matrix_left<rows,cols,rows,cols,rows,cols>);
  matrix_class->def("mul", matrix_by_matrix_right<rows,cols,rows,1,rows,1>);
  matrix_class->def("mul", matrix_by_matrix_dyn<rows,cols,Eigen::Dynamic,Eigen::Dynamic>);
  matrix_class->def("mul", matrix_by_matrix_right<rows,cols,Eigen::Dynamic,1,rows,1>);

  //matrix_class->def("mul", matrix_by_matrix<rows,cols>);
  def_common_matrix<rows,cols>(matrix_class);
  def_common_static<rows,cols>(matrix_class);
  def_common_pure_matrix<rows,cols>(matrix_class);
}

/** @brief wrap the Eigen::Matrix<Eigen::Dynamic,Eigen::Dynamic>
  */
template<int rows, int cols>
void def_dynamic_matrix(const char* name,const char* doc){
  class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >*matrix_class = new class_<TMATRIX, boost::noncopyable, boost::shared_ptr<TMATRIX > >(name,doc);
  matrix_class->def("__init__",make_constructor(&realmatrix_init_size_default<rows,cols>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_size_value<rows,cols>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_tab<rows,cols>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_dyn<4,4>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_dyn<3,3>));
  matrix_class->def("__init__",make_constructor(&realmatrix_init_copy_dyn<2,2>));
  matrix_class->def("setZero", dynamic_matrix_set_zero_default<rows,cols>);
  matrix_class->def("setOnes", dynamic_matrix_set_ones_default<rows,cols>);
  matrix_class->def("setConstant", dynamic_matrix_set_constant_default<rows,cols>);
  matrix_class->def("setRandom", dynamic_matrix_set_random_default<rows,cols>);
  matrix_class->def("resize", matrix_resize<rows,cols>);
  matrix_class->def("conservativeResize", matrix_conservative_resize<rows,cols>);
  matrix_class->def("identity", matrix_identity_dynamic<rows,cols>);
  matrix_class->def("mul", matrix_by_matrix_dyn<rows,cols,rows,cols>);
  matrix_class->def("mul", matrix_by_matrix_left<rows,cols,4,4,4,4>);
  matrix_class->def("mul", matrix_by_matrix_left<rows,cols,3,3,3,3>);
  matrix_class->def("mul", matrix_by_matrix_left<rows,cols,2,2,2,2>);
  matrix_class->def("mul", matrix_by_matrix_right<rows,cols,2,1,2,1>);
  matrix_class->def("mul", matrix_by_matrix_right<rows,cols,3,1,3,1>);
  matrix_class->def("mul", matrix_by_matrix_right<rows,cols,4,1,4,1>);
  matrix_class->def("mul", matrix_by_matrix_right<rows,cols,rows,1,rows,1>);
  def_common_matrix<rows,cols>(matrix_class);
  def_common_pure_matrix<rows,cols>(matrix_class);
  def_common_dynamic<rows,cols>(matrix_class);
}


/** @brief Define all matrix/vector types
  */
void def_matrix_types(){
  def_dynamic_matrix<Eigen::Dynamic,Eigen::Dynamic>("RealMatrix","Arbitrary sized matrix");
  def_static_matrix<2,2>("RealMatrix2","2x2 matrix");
  def_static_matrix<3,3>("RealMatrix3","3x3 matrix");
  def_static_matrix<4,4>("RealMatrix4","4x4 matrix");
  def_dynamic_vector<Eigen::Dynamic,1>("RealVector","Arbitrary sized vector");
  def_static_vector<1,1>("RealVector1","Vector (1x1 matrix)");
  def_static_vector<2,1>("RealVector2","Vector (2x1 matrix)");
  def_static_vector<3,1>("RealVector3","Vector (3x1 matrix)");
  def_static_vector<4,1>("RealVector4","Vector (4x1 matrix)");
}

} // python
} // cf3
