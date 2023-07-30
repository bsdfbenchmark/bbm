#ifndef _BBM_LITERAL_H_
#define _BBM_LITERAL_H_

/************************************************************************/
/*! \file literal.h

  \brief Work around for using floating point literals for compilers that
  do not support it (e.g., Clang 14).

*************************************************************************/

namespace bbm {

  template<typename T=int>
    struct literal
  {
    using type = T;
    
    //! \brief Constructor
    inline constexpr literal(T val=0) : value(val) {}
    
    //! \brief Cast to T
    inline constexpr operator T(void) const { return value; }
    
    //! \brief data
    T value;
  };

} // end bbm namespace

#endif /* _BBM_LITERAL_H_ */
