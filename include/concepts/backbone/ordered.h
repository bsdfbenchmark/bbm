#ifndef _BBM_ORDERED_BACKBONE_CONCEPT_H_
#define _BBM_ORDERED_BACKBONE_CONCEPT_H_

/************************************************************************/
/*! \file ordered.h

  \brief A valid ordering requires that the type has eq, neq, <, >, <=, and =>
  operators that each return the same type.  The return type does not need to
  be a boolean. == and != must return a boolean.
*************************************************************************/

namespace bbm {
  namespace concepts {
    namespace backbone {

      /*****************************************************************/
      /*! \brief Comparison operators:

          Requires that the type has eq, neq, <, >, <=, and => operators that
          each return the same type.  The return type does not need to be a
          boolean. == and != must return a boolean.
      ******************************************************************/
      template<typename T>
      concept ordered = requires(T a, T b)
      {
        { a == b } -> std::convertible_to<bool>;
        { a != b } -> std::convertible_to<bool>;
        { eq(a,b) };
        { neq(a,b) } -> std::same_as< decltype(eq(a,b)) >;
        { a < b } -> std::same_as< decltype(eq(a,b)) >;
        { a > b } -> std::same_as< decltype(eq(a,b)) >;
        { a <= b } -> std::same_as< decltype(eq(a,b)) >;
        { a >= b } -> std::same_as< decltype(eq(a,b)) >;
      };

    } // end backbone namespac
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_ORDERED_BACKBONE_CONCEPT_H_ */
