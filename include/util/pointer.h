#ifndef _BBM_POINTER_H_
#define _BBM_POINTER_H_

#include <ostream>
#include <memory>
#include "util/typestring.h"

/************************************************************************/
/*! \file pointer.h

  \brief Pointer wrapper that takes both shared and non-shared pointers.  If
  the pointer is 'managed' then the pointer is deleted when the reference
  count hits zero.  By default a pointer is not managed unless a shared ptr is
  assigned or it is explicitely marked as managed during construction.
************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief Pointer wrapper that takes handle both shared as well as regular
      (unmanaged) pointers.

      This is a wrapper around shared_ptr that replaced the deleter with an
      empty deleter if the object should not be managed by the pointer.
  *********************************************************************/
  template<typename T>
    class pointer : public std::shared_ptr<std::remove_reference_t<T>>
  {
    using ptr_t = std::add_pointer_t<std::remove_all_extents_t<T>>;
    using base_type = std::shared_ptr<std::remove_reference_t<T>>;
    static constexpr auto empty_delete = [](ptr_t){};
  public:

    //! \brief Init from a regular pointer
    inline constexpr pointer(ptr_t ptr=nullptr, bool managed=false) noexcept : base_type()
    {
      if(managed) this->reset(ptr);
      else this->reset(ptr, empty_delete);
    }

    //! @{ \name Init from a shared_ptr
    template<typename Y>
      inline constexpr pointer(const std::shared_ptr<Y>& ptr) noexcept : base_type(ptr) {}

    template<typename Y>
      inline constexpr pointer(std::shared_ptr<Y>&& ptr) noexcept : base_type(ptr) {}
    //! @}
    
    //! \brief Returns true if the pointer manages the object deallocation
    inline constexpr bool is_managed(void) const
    {
      auto del = std::get_deleter<decltype(empty_delete)>(*this);
      return !(del && (*del == empty_delete));
    }
  };
  
} // end bbm namespace


#endif /* _BBM_POINTER_H_ */
