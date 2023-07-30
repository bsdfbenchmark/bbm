#ifndef _BBM_PTR_WRAPPER_CONCEPT_H_
#define _BBM_PTR_WRAPPER_CONCEPT_H_

/************************************************************************/
/*! \file ptr_wrapper.h
  \brief A shared_ptr wrapper.
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief ptr_wrapper concept

      \tparam T = pointer wrapper class
      \tparam BASE = type of underlying wrapper object.
      
      A ptr wrapper has the following:
      + the arrow operator
      + const std::shared_ptr<BASE>& ptr(void) const
    *********************************************************************/
    template<typename T, typename BASE>
      concept ptr_wrapper = requires(const T& t)
    {
      { t.ptr() } -> std::convertible_to< std::shared_ptr<BASE> >;
      requires concepts::has_arrow<T>;
    };
  
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_PTR_WRAPPER_CONCEPT_H_ */
