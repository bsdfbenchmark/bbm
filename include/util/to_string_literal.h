#ifndef _BBM_TO_STRING_LITERAL_H_
#define _BBM_TO_STRING_LITERAL_H_

#include <concepts>
#include <cmath>
#include "util/string_literal.h"

/***********************************************************************/
/*! \file to_string_literal.h
  \brief convert other types to a string literal.
************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief convert integrals to string_literal

    \tparam N = (integer) literal to covert into a string_literal
    \tparam base = conversion base; must be less than 16. Default base=10.
   *********************************************************************/
  template<auto N, int base=10> requires std::integral<decltype(N)> && (base <= 16)
    constexpr auto to_string_literal(void)
  {
    // convert digit to chars
    constexpr char digits[17] = "0123456789ABCDEF";

    // if negative, add '-' sign
    if constexpr (N < 0) return string_literal("-") + to_string_literal<-N, base>();

    // if less than base; end recursion
    else if constexpr (N < base) return string_literal<2>({digits[N], '\0'});

    // otherwise, convert last digit, and recurse on N/base
    else return to_string_literal<N / base, base>() + to_string_literal<N % base, base>();
  }

} // end bbm namespace

#endif /* _BBM_TO_STRING_LITERAL_H_ */
