#ifndef _BBM_ENDIAN_H_
#define _BBM_ENDIAN_H_

#include <utility>
#include <cstdint>

/***********************************************************************/
/*! \file Endian.h
    \brief Detect Endianess of processor and conversion methods
************************************************************************/

namespace bbm {
  namespace endian {

    /*** Implementation Details ***/
    namespace detail {
      //! \brief Union to determine the Endianess.
      union magicNumberType
      {
        inline constexpr magicNumberType(void) : value16(1) {}

        uint16_t value16;
        uint8_t value8[2];
      };

      //! \brief Flag indicating whether the processor is little Endian
      static const bool _isLittleEndian = (magicNumberType().value8[0] == 1);

      //! \brief Helper method to swap the order from little to big or vice versa
      template<typename T>
      inline T swapOrder(const T& value)
      {
        // init
        T result;

        // get pointers
        void* result_p = static_cast<void*>(&result);
        const void* value_p = static_cast<const void*>(&value);

        const uint8_t* valueItr = static_cast<const uint8_t*>(value_p) + sizeof(T) - 1;
        uint8_t* resultItr = static_cast<uint8_t*>(result_p);
        uint8_t* resultEnd = resultItr + sizeof(T);
        
        // swap order
        for(; resultItr != resultEnd; valueItr--, resultItr++)
          *resultItr = *valueItr;

        // done.
        return result;
      } 
    } // end detail namespace
    

    //! @{ \name determine Endianess
    static const bool isLittleEndian = detail::_isLittleEndian; 
    static const bool isBigEndian =  !detail::_isLittleEndian;
    //! @}
    

    //! @{ \name Convert to Little Endian <=> Machine Representation
    template<typename T>
      inline T little(const T& value) 
      { 
        if(isLittleEndian) return value;
        else return detail::swapOrder(value);
      }
    
    template<typename Iterator>
      inline void little(const Iterator& begin, const Iterator& end)
      {
        if(isLittleEndian) return;
        for(Iterator itr=begin; itr != end; ++itr)
          *itr = detail::swapOrder(*itr);
      }
    //! @}

    
    //! @{ \name Convert Big Endian <=> Machine Representation
    template<typename T>
      inline T big(const T& value) 
      { 
        if(isBigEndian) return value;
        else return detail::swapOrder(value);
      }
    
    template<typename Iterator>
      inline void big(const Iterator& begin, const Iterator& end)
      {
        if(isBigEndian) return;
        for(Iterator itr=begin; itr != end; ++itr)
          *itr = detail::swapOrder(*itr);
      }
    //! @}
    
  } // end endian namespace
} // end bbm namespace

#endif /* _BBM_ENDIAN_H_ */
