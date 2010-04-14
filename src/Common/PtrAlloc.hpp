#ifndef CF_Common_PtrAlloc_hh
#define CF_Common_PtrAlloc_hh

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

namespace CF {

////////////////////////////////////////////////////////////////////////////////

///  Definition of CFNULL
#define CFNULL 0

////////////////////////////////////////////////////////////////////////////////

  /// @brief Deletes a pointer and makes sure it is set to CFNULL afterwards
  /// It would not have to check for CFNULL before deletion, as
  /// deleting a null is explicitely allowed by the standard.
  /// Nevertheless it does check, to avoid problems with not so compliant compilers.
  /// Do not use this function with data allocate with new [].
  /// @author Tiago Quintino
  /// @pre ptr has been allocated with operator new
  /// @param ptr pointer to be deleted
  /// @post  ptr equals CFNULL
  template <class TYPE>
  void delete_ptr (TYPE*& ptr)
  {
    if (ptr != CFNULL)
    {
      delete ptr;
      ptr = CFNULL;
    }
    cf_assert (ptr == CFNULL);
  }

  /// @brief Deletes a pointer and makes sure it is set to CFNULL afterwards
  /// It would not have to check for CFNULL before deletion, as
  /// deleting a null is explicitely allowed by the standard.
  /// Nevertheless it does check, to avoid problems with not so compliant compilers.
  /// Do not use this function with data allocate with new.
  /// @author Tiago Quintino
  /// @pre ptr has been allocated with operator new []
  /// @param ptr pointer to be deleted
  /// @post  ptr equals CFNULL
  template <class TYPE>
  void delete_ptr_array (TYPE*& ptr)
  {
    if (ptr != CFNULL)
    {
      delete [] ptr;
      ptr = CFNULL;
    }
    cf_assert (ptr == CFNULL);
  }

////////////////////////////////////////////////////////////////////////////////

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PtrAlloc_hh
