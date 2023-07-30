#ifndef _BBM_COLOR_H_
#define _BBM_COLOR_H_


/***********************************************************************/
/*! \file color.h

  \brief Defines additional helper methods for RGB colors

************************************************************************/

namespace bbm {
  namespace col {

    /********************************************************************/
    /*! @{ \name Shortcuts to color components
     ********************************************************************/
    template<typename T> inline constexpr decltype(auto) r(bbm::color<T>& c) { return c[0]; }
    template<typename T> inline constexpr decltype(auto) r(const bbm::color<T>& c) { return c[0]; }

    template<typename T> inline constexpr decltype(auto) g(bbm::color<T>& c) { return c[1]; }
    template<typename T> inline constexpr decltype(auto) g(const bbm::color<T>& c) { return c[1]; }

    template<typename T> inline constexpr decltype(auto) b(bbm::color<T>& c) { return c[2]; }
    template<typename T> inline constexpr decltype(auto) b(const bbm::color<T>& c) { return c[2]; }    
    //! @}

  } // end color namespace

} // end bbm namespace

#endif /* _BBM_COLOR_H_ */
