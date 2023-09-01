#ifndef _BBM_STRINGCONVERT_H_
#define _BBM_STRINGCONVERT_H_

#include <string>

/************************************************************************/
/*! \file stringconvert.h

  \brief concept to check if a type has a valid string_converter.
  
*************************************************************************/

namespace bbm {

  //! \brief forward decalaration
  template<typename T> struct string_converter;
  
  namespace concepts {

    /********************************************************************/
    /*! \brief concept to check if a type has a valid string_converter with:

      + std::string toString(const T& )
 
    *********************************************************************/
    template<typename T>
      concept to_string_converter = requires(const T& t)
    {
      { string_converter<T>::toString(t) } -> std::convertible_to<std::string>;
    };

    /********************************************************************/
    /*! \brief concept to check if a type has a valid string_converter with:

      + T fromString(const std::string& )

    *********************************************************************/
    template<typename T>
      concept from_string_converter = requires(const T& t)
    {
      { string_converter<T>::fromString(std::declval<std::string>()) } -> std::same_as<T>;
    };

    /********************************************************************/
    /*! \brief Concept to check if type has a toString member
     ********************************************************************/
    template<typename T>
      concept has_toString = requires(T t)
    {
      { t.toString() } -> std::same_as<std::string>;
    };

    /********************************************************************/
    /*! \brief Concept to check if type has a fromString member
     ********************************************************************/
    template<typename T>
      concept has_fromString = requires
    {
      { std::decay_t<T>::fromString( std::declval<std::string>() ) } -> std::same_as<std::decay_t<T>>;
    };


    /********************************************************************/
    /*! @{ \name check if string conversion is fully defined
      *******************************************************************/
    template<typename T> concept to_stringconvert = (has_toString<T> || to_string_converter<T>);
    template<typename T> concept from_stringconvert = (has_fromString<T> || from_string_converter<T>);
    template<typename T> concept stringconvert = to_stringconvert<T> && from_stringconvert<T>;
    //! @}
    
  } // end concepts namespace


  /**********************************************************************/
  /*! @{ \name forward declarations of toString and fromString
   **********************************************************************/
  template<typename T> requires concepts::from_stringconvert<T>
    inline T fromString(const std::string&);

  template<typename T> requires concepts::to_stringconvert<T>
    inline std::string toString(const T&);  

} // end bbm namespace

#endif /* _BBM_STRINGCONVERT_H_ */
