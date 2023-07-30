#ifndef _BBM_CONCEPT_MACRO_H_
#define _BBM_CONCEPT_MACRO_H_

#include <concepts>
#include "core/error.h"
#include "util/macro_util.h"
#include "concepts/config.h"

/***********************************************************************/
/*! \file macro.h

  \brief Macros for checking if a class meets a concept.

  Examples:
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  BBM_CHECK_RAW_CONCEPT( std::constructible_from, std::complex<float>, float, float );
  BBM_CHECK_CONCEPT( std::constructible_from, bsdf_ptr<config>, Value_t<config> );
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  The first checks if std::complex<float> is constructible from 2 floats,
  i.e., std::constructible_from< vec2d<float>, float, float> is true.  This
  check can be used in evaluated and unevaluated contexts.

  The second checks if bsdf_ptr<config> is constribile from a Value_t<config>
  where config is a archetypical bbm configuration.  BBM_CHECK_CONCEPT ensure
  that bbm::concepts::archetype is in the namespace. This check can only be
  used in an unevaluated context.
  
************************************************************************/

//! \brief Check a class for a concept
#define BBM_CHECK_RAW_CONCEPT(CONCEPTNAME, CLASSNAME, ...) static_assert( CONCEPTNAME<CLASSNAME __VA_OPT__(,) __VA_ARGS__>, BBM_ERROR_MSG(#CLASSNAME does not meet #CONCEPTNAME) );

//! \brief Check a class for a concept with  bbm::concepts::archetypes in the namespace
#define BBM_CHECK_CONCEPT(CONCEPTNAME, CLASSNAME, ...)                   \
  template<string_literal, size_t> void concept_check(void);             \
  template<> void concept_check<__FILE__,__LINE__>(void)                 \
  {                                                                      \
    using namespace bbm::concepts::archetype;                            \
    BBM_CHECK_RAW_CONCEPT(CONCEPTNAME, CLASSNAME __VA_OPT__(,) __VA_ARGS__); \
  }                                                                      \

#endif /* _BBM_CONCEPT_MACRO_H_ */
