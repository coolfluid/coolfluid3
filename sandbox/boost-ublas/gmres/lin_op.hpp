//          Copyright Gunter Winkler 2004 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// authors: Gunter Winkler <guwi17 at gmx dot de>
//          Konstantin Kutzkow


#ifndef __INCLUDE_LIN_OP_HPP__
#define __INCLUDE_LIN_OP_HPP__


/** \brief a matrix based linear operator mapping a vector x (preimage) to a vector y (image).
* - \param M is the type of the matrix A
*/
template < class MATRIX >
class LinOp
{
public:
    /// general operator interface
    template < class VEC >
    VEC operator()(const VEC & x) const {
        VEC y( image_size() );
        mult(x,y);
        return y;
    }

    /// compute y <- Ax
    template < class VEC1, class VEC2 >
    void mult(const VEC1 & x, VEC2 & y) const {
        y = prod(A, x);
    }
    
    /// compute y <- b - Ax
    template < class VEC1, class VEC2, class VEC3 >
    void residuum(const VEC1 & x, const VEC2 & b, VEC3 & y) const {
        y = b - prod(A, x);
    }
    
    std::size_t image_size() const {
        return A.size1();
    }
    
    std::size_t preimage_size() const {
        return A.size2();
    }

    /// constructor from a matrix
    LinOp(MATRIX const & matrix) : A(matrix) { }
        
private:
    MATRIX const & A;
};


#endif



