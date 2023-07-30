#ifndef _BBM_FLAGS_H_
#define _BBM_FLAGS_H_

#include <type_traits>

/***********************************************************************/
/*! \file flags.h
    \brief Scoped enum operators and methods

  The defined operators are:

  + is_set(val, flag): Check if all in 'flag' are set in 'val.  Note this is
  not a symmetric operation.
  + operator| : performs OR operation of flags.  __NOTE__: the combined flag
  must be defined in the enum body.
  + operator& : performs AND operation on flags.
  + operator^ : XOR operator
  + operator~ : NOT operator

  Guarantees 'constexpr' when is_enum_v<FLAGNAME>
  
*************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! @{ \name Core Enum Operations 
   *********************************************************************/
  
  //! \brief Concat two flags
  template<typename FLAGNAME> requires std::is_enum_v<FLAGNAME>
    inline constexpr FLAGNAME operator|(FLAGNAME a, FLAGNAME b)
  {
    return FLAGNAME(std::underlying_type_t<FLAGNAME>(a) | std::underlying_type_t<FLAGNAME>(b));
  }

  //! \brief Conact two flags
  template<typename FLAGNAME> requires std::is_enum_v<FLAGNAME>
    inline constexpr FLAGNAME operator+(FLAGNAME a, FLAGNAME b)
  {
    return FLAGNAME(std::underlying_type_t<FLAGNAME>(a) | std::underlying_type_t<FLAGNAME>(b));
  }

  //! \brief Get the shared flags
  template<typename FLAGNAME> requires std::is_enum_v<FLAGNAME>
    inline constexpr FLAGNAME operator&(FLAGNAME a, FLAGNAME b)
  {
    return FLAGNAME(std::underlying_type_t<FLAGNAME>(a) & std::underlying_type_t<FLAGNAME>(b));
  }

  //! \brief Get the flags from 'a' that are not in 'b'
  template<typename FLAGNAME> requires std::is_enum_v<FLAGNAME>
    inline constexpr FLAGNAME operator^(FLAGNAME a, FLAGNAME b)
  {
    return FLAGNAME(std::underlying_type_t<FLAGNAME>(a) ^ std::underlying_type_t<FLAGNAME>(b));
  }

  //! \brief Set/unset flag that are unset/set respectively.
  template<typename FLAGNAME> requires std::is_enum_v<FLAGNAME>
    inline constexpr FLAGNAME operator~(FLAGNAME a)
  {
    return FLAGNAME(~std::underlying_type_t<FLAGNAME>(a));
  }

  //! \brief Update 'a' with a & b
  template<typename FLAGNAME> requires std::is_enum_v<FLAGNAME>
    inline constexpr FLAGNAME& operator&=(FLAGNAME& a, FLAGNAME b)
  {
    a = a & b;
    return a;
  }

  //! \brief Update 'a' with a | b
  template<typename FLAGNAME> requires std::is_enum_v<FLAGNAME>
    inline constexpr FLAGNAME& operator|=(FLAGNAME& a, FLAGNAME b)
  {
    a = a | b;
    return a;
  }

  //! \brief Update 'a' with a+b
  template<typename FLAGNAME> requires std::is_enum_v<FLAGNAME>
    inline constexpr FLAGNAME& operator+=(FLAGNAME& a, FLAGNAME b)
  {
    a = a | b;
    return a;
  }

  //! \brief Update 'a' with a^b,
  template<typename FLAGNAME> requires std::is_enum_v<FLAGNAME>
  inline constexpr FLAGNAME& operator^=(FLAGNAME& a, FLAGNAME b)
  {
    a = a ^ b;
    return a;
  }

  //! \brief Check if all in 'flag' are also set in 'a'; compatible with packet types.
  template<typename FLAGNAME, typename FLAG> requires std::is_enum_v<FLAG> && std::is_same_v<scalar_t<FLAGNAME>, FLAG>
    inline constexpr auto is_set(const FLAGNAME& a, const FLAG& flag)
  {
    // Ensure constexpr when FLAGNAME == FLAG
    if constexpr (std::is_same_v<FLAGNAME, FLAG>) return ((a & flag) == flag);

    // Otherwise use eq function (does not guarantee constexpr)
    else return eq((a & flag), flag);
  }

  //! @}

} // end bbm namespace

#endif /* _BBM_FLAGS_H_ */
