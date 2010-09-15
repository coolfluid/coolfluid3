// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "SVDInverter.hpp"
#include "MathChecks.hpp"
#include "Common/Log.hpp"
#include "Common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  using namespace Common;
  
  namespace Math {

////////////////////////////////////////////////////////////////////////////////

SVDInverter::SVDInverter(const Uint& nbRows, const Uint& nbCols) :
  MatrixInverter(),
  m_rows(nbRows),
  m_cols(nbCols),
  m_u(nbRows,nbCols),
  m_v(nbCols,nbCols),
  m_s(nbCols),
  m_decomposed(false)
{
}

////////////////////////////////////////////////////////////////////////////////

SVDInverter::SVDInverter(const RealMatrix& a) :
  MatrixInverter(),
  m_rows(a.nbRows()),
  m_cols(a.nbCols()),
  m_u(a),
  m_v(m_cols,m_cols),
  m_s(m_cols),
  m_decomposed(false)
{
  try {
    decompose();
  }
  catch(Common::ConvergenceNotReached& e) {
    CFLogInfo(e.what() << "\n");
    throw;
  }
  reorder();
}

////////////////////////////////////////////////////////////////////////////////

SVDInverter::~SVDInverter()
{
}

////////////////////////////////////////////////////////////////////////////////

void SVDInverter::invert(const RealMatrix& a, RealMatrix& x)
{
  if (!m_decomposed) {
    m_u = a;
    decompose();
    reorder();
}

  invert(x);
}

////////////////////////////////////////////////////////////////////////////////

void SVDInverter::invert(RealMatrix& x)
{
  // cf_assert(m_decomposed == true);

Real tsh = 0.5*sqrt(m_rows+m_cols+1.)*m_s[0]*MathConsts::RealEps();

  RealVector invS(m_cols);
  for (Uint j=0; j<m_cols; ++j) {
    if (m_s[j] > tsh) {
    invS[j]=1./m_s[j];
  } else {
      invS[j]=0.0;
  }
  }

  RealMatrix transU(m_cols,m_rows);
  m_u.transpose(transU);

  if (x.nbRows()!=m_cols || x.nbCols()!=m_rows){
    x.resize(m_cols,m_rows);
  }
  // Moore-Penrose Inverse  pinv(A) = V*inv(S)*U'
  x = m_v * (invS * transU);
}

////////////////////////////////////////////////////////////////////////////////

void SVDInverter::solve(const RealMatrix& a, const RealVector& b, RealVector& x, Real thresh)
{
  if (!m_decomposed) {
    decompose(a);
}
  solve(b,x,thresh);
}

////////////////////////////////////////////////////////////////////////////////

void SVDInverter::solve(const RealMatrix& a, const RealMatrix& b, RealMatrix& x, Real thresh)
{
Uint i,j,m=b.nbCols();
cf_assert(b.nbRows() == m_cols && x.nbRows() == m_cols && b.nbCols() == x.nbCols());
RealVector xx(m_cols);
for (j=0;j<m;j++) {
  for (i=0;i<m_cols;i++) xx[i] = b(i,j);
  solve(a,xx,xx,thresh);
  for (i=0;i<m_cols;i++) x(i,j) = xx[i];
}
}

////////////////////////////////////////////////////////////////////////////////

void SVDInverter::solve(const RealVector& b, RealVector& x, Real thresh) {

Uint i,j,jj;
Real s;
cf_assert(b.size() == m_rows && x.size() == m_cols);
RealVector tmp(m_cols);
Real tsh = (thresh >= 0. ? thresh : 0.5*sqrt(m_rows+m_cols+1.)*m_s[0]*MathConsts::RealEps());
for (j=0;j<m_cols;j++) {
  s=0.0;
  if (m_s[j] > tsh) {
  for (i=0;i<m_rows;i++) s += m_u(i,j)*b[i];
  s /= m_s[j];
  }
  tmp[j]=s;
}
for (j=0;j<m_cols;j++) {
  s=0.0;
  for (jj=0;jj<m_cols;jj++) s += m_v(j,jj)*tmp[jj];
  x[j]=s;
}
}

////////////////////////////////////////////////////////////////////////////////

void SVDInverter::solve(const RealMatrix& b, RealMatrix& x, Real thresh)
{
Uint i,j,m=b.nbCols();
cf_assert(b.nbRows() == m_cols && x.nbRows() == m_cols && b.nbCols() == x.nbCols());
RealVector xx(m_cols);
for (j=0;j<m;j++) {
  for (i=0;i<m_cols;i++) xx[i] = b(i,j);
  solve(xx,xx,thresh);
  for (i=0;i<m_cols;i++) x(i,j) = xx[i];
}
}

////////////////////////////////////////////////////////////////////////////////

Uint SVDInverter::rank(Real thresh = -1.) {
Uint j,nr=0;
Real tsh = (thresh >= 0. ? thresh : 0.5*sqrt(m_rows+m_cols+1.)*m_s[0]*MathConsts::RealEps());
for (j=0;j<m_cols;j++) if (m_s[j] > tsh) nr++;
return nr;
}

////////////////////////////////////////////////////////////////////////////////

Uint SVDInverter::nullity(Real thresh = -1.) {
Uint j,nn=0;
Real tsh = (thresh >= 0. ? thresh : 0.5*sqrt(m_rows+m_cols+1.)*m_s[0]*MathConsts::RealEps());
for (j=0;j<m_cols;j++) if (m_s[j] <= tsh) nn++;
return nn;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix SVDInverter::range(Real thresh = -1.){
Uint i,j,nr=0;
Real tsh = (thresh >= 0. ? thresh : 0.5*sqrt(m_rows+m_cols+1.)*m_s[0]*MathConsts::RealEps());
RealMatrix rnge(m_rows,rank(thresh));
for (j=0;j<m_cols;j++) {
  if (m_s[j] > tsh) {
  for (i=0;i<m_rows;i++) rnge(i,nr) = m_u(i,j);
  nr++;
  }
}
return rnge;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix SVDInverter::nullspace(Real thresh = -1.){
Uint j,jj,nn=0;
Real tsh = (thresh >= 0. ? thresh : 0.5*sqrt(m_rows+m_cols+1.)*m_s[0]*MathConsts::RealEps());
RealMatrix nullsp(m_cols,nullity(thresh));
for (j=0;j<m_cols;j++) {
  if (m_s[j] <= tsh) {
  for (jj=0;jj<m_cols;jj++) nullsp(jj,nn) = m_v(jj,j);
  nn++;
  }
}
return nullsp;
}

////////////////////////////////////////////////////////////////////////////////

void SVDInverter::decompose(const RealMatrix& a) {
  m_u = a;
  decompose();
  reorder();
}

//////////////////////////////////////////////////////////////////////////////

void SVDInverter::decompose() {
bool flag;

int i=0,its=0,j=0,jj=0,k=0,l=0,nm=0;
  int rows(m_rows), cols(m_cols); //conversion from Uint to int
Real anorm,c,f,g,h,s,scale,x,y,z;
RealVector rv1(m_cols);
g = scale = anorm = 0.0;
for (i=0;i<cols;i++) {
  l=i+2;
  rv1[i]=scale*g;
  g=s=scale=0.0;
  if (i < rows) {
  for (k=i;k<rows;k++) scale += std::abs(m_u(k,i));
  if (scale != 0.0) {
    for (k=i;k<rows;k++) {
    m_u(k,i) /= scale;
    s += m_u(k,i)*m_u(k,i);
    }
    f=m_u(i,i);
    g = -MathFunctions::changeSign(sqrt(s),f);
    h=f*g-s;
    m_u(i,i)=f-g;
    for (j=l-1;j<cols;j++) {
    for (s=0.0,k=i;k<rows;k++) s += m_u(k,i)*m_u(k,j);
    f=s/h;
    for (k=i;k<rows;k++) m_u(k,j) += f*m_u(k,i);
    }
    for (k=i;k<rows;k++) m_u(k,i) *= scale;
  }
  }
  m_s[i]=scale *g;
  g=s=scale=0.0;
  if (i+1 <= rows && i+1 != cols) {
  for (k=l-1;k<cols;k++) scale += std::abs(m_u(i,k));
  if (scale != 0.0) {
    for (k=l-1;k<cols;k++) {
    m_u(i,k) /= scale;
    s += m_u(i,k)*m_u(i,k);
    }
    f=m_u(i,l-1);
    g = -MathFunctions::changeSign(sqrt(s),f);
    h=f*g-s;
    m_u(i,l-1)=f-g;
    for (k=l-1;k<cols;k++) rv1[k]=m_u(i,k)/h;
    for (j=l-1;j<rows;j++) {
    for (s=0.0,k=l-1;k<cols;k++) s += m_u(j,k)*m_u(i,k);
    for (k=l-1;k<cols;k++) m_u(j,k) += s*rv1[k];
    }
    for (k=l-1;k<cols;k++) m_u(i,k) *= scale;
  }
  }
  anorm=std::max(anorm,(std::abs(m_s[i])+std::abs(rv1[i])));
}
for (i=m_cols-1;i>=0;i--) {
  if (i < cols-1) {
  if (g != 0.0) {
    for (j=l;j<cols;j++) {
    m_v(j,i)=(m_u(i,j)/m_u(i,l))/g;
    }
    for (j=l;j<cols;j++) {
    for (s=0.0,k=l;k<cols;k++) s += m_u(i,k)*m_v(k,j);
    for (k=l;k<cols;k++) m_v(k,j) += s*m_v(k,i);
    }
  }
  for (j=l;j<cols;j++) m_v(i,j)=m_v(j,i)=0.0;
  }
  m_v(i,i)=1.0;
  g=rv1[i];
  l=i;
}
for (i=std::min(m_rows,m_cols)-1;i>=0;i--) {
  l=i+1;
  g=m_s[i];
  for (j=l;j<cols;j++) m_u(i,j)=0.0;
  if (g != 0.0) {
  g=1.0/g;
  for (j=l;j<cols;j++) {
    for (s=0.0,k=l;k<rows;k++) s += m_u(k,i)*m_u(k,j);
    f=(s/m_u(i,i))*g;
    for (k=i;k<rows;k++) m_u(k,j) += f*m_u(k,i);
  }
  for (j=i;j<rows;j++) m_u(j,i) *= g;
  } else for (j=i;j<rows;j++) m_u(j,i)=0.0;
  ++m_u(i,i);
}
for (k=m_cols-1;k>=0;k--) {
  for (its=0;its<30;its++) {
  flag=true;
  for (l=k;l>=0;l--) {
    nm=l-1;
    if (l == 0 || std::abs(rv1[l]) <= MathConsts::RealEps()*anorm) {
    flag=false;
    break;
    }
    if (std::abs(m_s[nm]) <= MathConsts::RealEps()*anorm) break;
  }
  if (flag) {
    c=0.0;
    s=1.0;
    for (i=l;i<k+1;i++) {
    f=s*rv1[i];
    rv1[i]=c*rv1[i];
    if (std::abs(f) <= MathConsts::RealEps()*anorm) break;
    g=m_s[i];
    h=pythag(f,g);
    m_s[i]=h;
    h=1.0/h;
    c=g*h;
    s = -f*h;
    for (j=0;j<rows;j++) {
      y=m_u(j,nm);
      z=m_u(j,i);
      m_u(j,nm)=y*c+z*s;
      m_u(j,i)=z*c-y*s;
    }
    }
  }
  z=m_s[k];
  if (l == k) {
    if (z < 0.0) {
    m_s[k] = -z;
    for (j=0;j<cols;j++) m_v(j,k) = -m_v(j,k);
    }
    break;
  }
  if (its == 29) throw ConvergenceNotReached(FromHere(),"no convergence in 30 svdcmp iterations");
  x=m_s[l];
  nm=k-1;
  y=m_s[nm];
  g=rv1[nm];
  h=rv1[k];
  f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
  g=pythag(f,1.0);
  f=((x-z)*(x+z)+h*((y/(f+MathFunctions::changeSign(g,f)))-h))/x;
  c=s=1.0;
  for (j=l;j<=nm;j++) {
    i=j+1;
    g=rv1[i];
    y=m_s[i];
    h=s*g;
    g=c*g;
    z=pythag(f,h);
    rv1[j]=z;
    c=f/z;
    s=h/z;
    f=x*c+g*s;
    g=g*c-x*s;
    h=y*s;
    y *= c;
    for (jj=0;jj<cols;jj++) {
    x=m_v(jj,j);
    z=m_v(jj,i);
    m_v(jj,j)=x*c+z*s;
    m_v(jj,i)=z*c-x*s;
    }
    z=pythag(f,h);
    m_s[j]=z;
    if (z) {
    z=1.0/z;
    c=f*z;
    s=h*z;
    }
    f=c*g+s*y;
    x=c*y-s*g;
    for (jj=0;jj<rows;jj++) {
    y=m_u(jj,j);
    z=m_u(jj,i);
    m_u(jj,j)=y*c+z*s;
    m_u(jj,i)=z*c-y*s;
    }
  }
  rv1[l]=0.0;
  rv1[k]=f;
  m_s[k]=x;
  }
}
  m_decomposed = true;

}

////////////////////////////////////////////////////////////////////////////////

void SVDInverter::reorder() {
Uint i,j,k,s,inc=1;
Real sw;
RealVector su(m_rows), sv(m_cols);
do { inc *= 3; inc++; } while (inc <= m_cols);
do {
  inc /= 3;
  for (i=inc;i<m_cols;i++) {
  sw = m_s[i];
  for (k=0;k<m_rows;k++) su[k] = m_u(k,i);
  for (k=0;k<m_cols;k++) sv[k] = m_v(k,i);
  j = i;
  while (m_s[j-inc] < sw) {
    m_s[j] = m_s[j-inc];
    for (k=0;k<m_rows;k++) m_u(k,j) = m_u(k,j-inc);
    for (k=0;k<m_cols;k++) m_v(k,j) = m_v(k,j-inc);
    j -= inc;
    if (j < inc) break;
  }
  m_s[j] = sw;
  for (k=0;k<m_rows;k++) m_u(k,j) = su[k];
  for (k=0;k<m_cols;k++) m_v(k,j) = sv[k];

  }
} while (inc > 1);
for (k=0;k<m_cols;k++) {
  s=0;
  for (i=0;i<m_rows;i++) if (m_u(i,k) < 0.) s++;
  for (j=0;j<m_cols;j++) if (m_v(j,k) < 0.) s++;
  if (s > (m_rows+m_cols)/2) {
  for (i=0;i<m_rows;i++) m_u(i,k) = -m_u(i,k);
  for (j=0;j<m_cols;j++) m_v(j,k) = -m_v(j,k);
  }
}
}

////////////////////////////////////////////////////////////////////////////////

Real SVDInverter::pythag(const Real a, const Real b) {
Real absa=std::abs(a), absb=std::abs(b);
return (absa > absb ? absa*std::sqrt(1.0+ (absb/absa)*(absb/absa) ) :
  (absb == 0.0 ? 0.0 : absb*std::sqrt(1.0+ (absa/absb)*(absa/absb) )));
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////
