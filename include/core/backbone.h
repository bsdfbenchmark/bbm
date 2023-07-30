#ifndef _BBM_BACKBONE_H_
#define _BBM_BACKBONE_H_

#include "concepts/util.h"

#include "util/tuple.h"
#include "util/named.h"
#include "util/toString.h"
#include "util/reflection.h"
#include "util/apply_all.h"
#include "core/attribute.h"

/************************************************************************/
/*! \file backbone.h

  \brief Active the current backbone.

************************************************************************/

/************************************************************************/
/*! \brief BBM_BACKBONE_IMPORT: will be called each time BBM_IMPORT_CONFIG is called; default do nothing
*************************************************************************/
#ifndef BBM_BACKBONE_IMPORT
  #define BBM_BACKBONE_IMPORT(Config)
#endif

/************************************************************************/
/*! \brief Helper Macro to validate the backbone for a given type
  + TYPE has basic_math (+, -, *, /, %)
  + TYPE supports (==, !=, <, >, <=, and >=)
  + TYPE supports horizontal operations
  + TYPE supports the control method select, lookup, and detach.
  + mask_t<TYPE> is valid (bit and logical operators)
  + replace_scalar_t<TYPE> is a valid size_t.
*************************************************************************/
#define BBM_VALIDATE_BACKBONE(TYPE)                                      \
  BBM_CHECK_RAW_CONCEPT( std::constructible_from, TYPE );                \
  BBM_CHECK_RAW_CONCEPT( std::copy_constructible, TYPE );                \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::has_basic_math, TYPE );          \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::backbone::has_math_functions, TYPE ); \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::backbone::ordered, TYPE );       \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::backbone::horizontal, TYPE );    \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::backbone::control, TYPE );       \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::backbone::is_rng, rng<TYPE> );   \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::has_logical_operators, bbm::mask_t<TYPE> ); \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::has_bit_operators, bbm::mask_t<TYPE> ); \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::has_basic_math, bbm::replace_scalar_t<TYPE, size_t>); \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::has_increment, bbm::replace_scalar_t<TYPE, size_t> ); \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::has_decrement, bbm::replace_scalar_t<TYPE, size_t> ); \
  BBM_CHECK_RAW_CONCEPT( bbm::concepts::backbone::if_diff_gradient, TYPE ); \


/*** Forward Declarations for gradient ***/
namespace backbone {
  template<typename T> remove_diff_t<T>& detach_gradient(T&);
  template<typename T> void track_gradient(T&, bool);
  template<typename T> bool is_gradient_tracked(T&);
  template<typename T> const remove_diff_t<T>& gradient(T&);
  template<typename T> void forward_gradient(T&);
  template<typename T> void backward_gradient(T&);

  std::string toString(void) { return ""; }
}

//////////////////////////////////////////////////////////////////////////
namespace bbm {

  //! @{ \name type_traits
  using backbone::is_diff_v;
  using backbone::is_mask_v;
  using backbone::is_index_v;
  using backbone::is_packet_v;
  
  using backbone::remove_diff_t;
  using backbone::add_diff_t;
  using backbone::remove_packet_t;
  using backbone::add_packet_t;
  using backbone::mask_t;
  using backbone::index_t;
  using backbone::index_mask_t;
  using backbone::value_t;
  using backbone::scalar_t;
  using backbone::replace_scalar_t;
  //! @}
  
  //! @{ \name import core types
  using backbone::vec2d;
  using backbone::vec3d;
  using backbone::color;
  using backbone::complex;
  //! @}

  //! \brief Import custom toString
  using backbone::toString;

  //! @{ \name random number generator
  using backbone::seed_t;
  using backbone::default_seed;
  using backbone::rng;
  //! @}
  
  /**********************************************************************/
  /*! \brief Helper macro: extend existing methods to support attribute types
   **********************************************************************/
#define BBM_CALL_BACKBONE_OP(OpName)                                     \
  template<typename... T> requires requires(const T&... t) {{backbone::OpName(bbm::value(t)...)};} \
    inline constexpr auto OpName(const T&... t)                          \
  {                                                                      \
    return backbone::OpName(bbm::value(t)...);                           \
  }                                                                      \
  
  //! @{ \name math functions
  BBM_CALL_BACKBONE_OP(cossin);
  BBM_CALL_BACKBONE_OP(eq);
  BBM_CALL_BACKBONE_OP(neq);
  BBM_CALL_BACKBONE_OP(isnan);
  BBM_CALL_BACKBONE_OP(isinf);
  BBM_CALL_BACKBONE_OP(isfinite);
  //! @}

  //! @{ \name complex functions
  BBM_CALL_BACKBONE_OP(real);
  BBM_CALL_BACKBONE_OP(imag);
  BBM_CALL_BACKBONE_OP(conj);
  //! @}
  
  //! @{ \name horizontal methods
  BBM_CALL_BACKBONE_OP(hsum);
  BBM_CALL_BACKBONE_OP(hprod);
  BBM_CALL_BACKBONE_OP(hmax);
  BBM_CALL_BACKBONE_OP(hmin);
  BBM_CALL_BACKBONE_OP(dot);
  BBM_CALL_BACKBONE_OP(norm);
  BBM_CALL_BACKBONE_OP(squared_norm);
  BBM_CALL_BACKBONE_OP(normalize);
  BBM_CALL_BACKBONE_OP(all);
  BBM_CALL_BACKBONE_OP(any);
  BBM_CALL_BACKBONE_OP(none);
  BBM_CALL_BACKBONE_OP(count);
  //! @}

  #undef BBM_CALL_BACKBONE_OP
  
  //! @{ \name control methods
  using backbone::cast;
  using backbone::binary_search;


  /**********************************************************************/
  /* \brief Helper Macro for extending backbone methods to support tuples
             named, and unnamed accessible types.
  **********************************************************************/
#define BBM_CALL_BACKBONE_FUNC(FuncName)                                 \
  template<typename T, typename... U> requires requires(const T& t, const U&... u) {{apply_all([](auto& t, auto&... u) { return backbone::FuncName(t, u...); }, t, u...)};} \
    inline auto FuncName (const T& t, const U&... u)                     \
  {                                                                      \
    return apply_all( [](auto& t, auto&... u) { return backbone::FuncName(t, u...); }, t, u...); \
  }                                                                      \

  //! @{ Math functions (only those that return the same type are extended to tupe/named/unnamed)
  BBM_CALL_BACKBONE_FUNC( exp )
  BBM_CALL_BACKBONE_FUNC( log )
  BBM_CALL_BACKBONE_FUNC( sin )
  BBM_CALL_BACKBONE_FUNC( asin )
  BBM_CALL_BACKBONE_FUNC( cos )
  BBM_CALL_BACKBONE_FUNC( acos )
  BBM_CALL_BACKBONE_FUNC( tan )
  BBM_CALL_BACKBONE_FUNC( atan )
  BBM_CALL_BACKBONE_FUNC( atan2 )
  BBM_CALL_BACKBONE_FUNC( sinh )
  BBM_CALL_BACKBONE_FUNC( asinh )
  BBM_CALL_BACKBONE_FUNC( cosh )
  BBM_CALL_BACKBONE_FUNC( acosh )
  BBM_CALL_BACKBONE_FUNC( tanh )
  BBM_CALL_BACKBONE_FUNC( atanh )
  BBM_CALL_BACKBONE_FUNC( ceil )
  BBM_CALL_BACKBONE_FUNC( floor )
  BBM_CALL_BACKBONE_FUNC( round )
  BBM_CALL_BACKBONE_FUNC( clamp )
  BBM_CALL_BACKBONE_FUNC( fmod )
  BBM_CALL_BACKBONE_FUNC( copysign )
  BBM_CALL_BACKBONE_FUNC( sign )
  BBM_CALL_BACKBONE_FUNC( lerp )
  BBM_CALL_BACKBONE_FUNC( abs )
  BBM_CALL_BACKBONE_FUNC( sqrt )
  BBM_CALL_BACKBONE_FUNC( cbrt )
  BBM_CALL_BACKBONE_FUNC( pow )
  BBM_CALL_BACKBONE_FUNC( max )
  BBM_CALL_BACKBONE_FUNC( min )
  BBM_CALL_BACKBONE_FUNC( safe_sqrt )
  BBM_CALL_BACKBONE_FUNC( safe_asin )
  BBM_CALL_BACKBONE_FUNC( safe_acos )
  BBM_CALL_BACKBONE_FUNC( erf )
  BBM_CALL_BACKBONE_FUNC( erfc )
  BBM_CALL_BACKBONE_FUNC( erfinv )
  BBM_CALL_BACKBONE_FUNC( tgamma )
  BBM_CALL_BACKBONE_FUNC( lgamma )
  BBM_CALL_BACKBONE_FUNC( rcp )
  BBM_CALL_BACKBONE_FUNC( rsqrt )
  //! @}
  
  #undef BBM_CALL_BACKBONE_FUNC

  
  namespace detail {
    /********************************************************************/
    /*! \brief Generalized 'select' A or B based on the MASK.

      \param mask = selection mask; true => select A; otherwise B.
      \param a = select if true
      \param b = select if false

      If A and B are tuples, named tuples, or support reflection, then
      perform selection on all elements based on binary mask (i.e., all
      elements of A are copied, or all elements of B are selected).
    *********************************************************************/
    template<typename MASK, typename A, typename B> requires (concepts::reflection::supported<A> && concepts::reflection::supported<B>) || (is_tuple_v<anonymize_t<A>> && is_tuple_v<anonymize_t<B>>) || requires(MASK m, A a, B b) {{backbone::select(m,a,b)};}
      inline constexpr auto select(MASK&& mask, A&& a, B&& b)
    {
      // Case 1: tuples
      if constexpr (is_tuple_v<A> && is_tuple_v<B>)
      {
        static_assert(std::tuple_size_v<std::decay_t<A>> == std::tuple_size_v<std::decay_t<B>>, BBM_SIZE_MISMATCH);
        auto select_helper = [&]<size_t... IDX>(std::index_sequence<IDX...>) { return make_ref_tuple( bbm::detail::select(std::forward<MASK>(mask), std::get<IDX>(std::forward<A>(a)), std::get<IDX>(std::forward<B>(b)))... ); };
        return select_helper(std::make_index_sequence<std::tuple_size_v<std::decay_t<A>>>{});
      }

      // Case 2: named tuples
      else if constexpr (named_equivalence_v<A,B>)
      {
        auto select_helper = [&]<size_t... IDX>(std::index_sequence<IDX...>) { return make_named< std::decay_t<A>::template name<IDX>... >( make_ref_tuple( bbm::detail::select(std::forward<MASK>(mask), bbm::get<std::decay_t<A>::template name<IDX>>(a), bbm::get<std::decay_t<A>::template name<IDX>>(b))... ) ); };
        return select_helper(std::make_index_sequence<std::tuple_size_v<std::decay_t<A>>>{});
      }
      else if constexpr (is_named_v<A> != is_named_v<B>)
      {
        auto select_helper = [&]<typename T, string_literal... NAMES>(named<T,NAMES...>) { return make_named<NAMES...>( bbm::detail::select(mask, anonymize_v(a), anonymize_v(b) ) ); };
        if constexpr (is_named_v<A>) return select_helper(a);
        else if constexpr (is_named_v<B>) return select_helper(b);
      }
      
      // Case 3: supports refllection
      else if constexpr (concepts::reflection::supported<A> && concepts::reflection::supported<B>)
      {
        using result_t = std::conditional_t<std::is_convertible_v<std::decay_t<A>, std::decay_t<B>>, std::decay_t<B>, std::decay_t<A>>;
        result_t result;
        reflection::attributes(result) = bbm::detail::select(std::forward<MASK>(mask), reflection::attributes(std::forward<A>(a)), reflection::attributes(std::forward<B>(b)));
        return result;
      }
      
      // Otherwise forward to backbone
      else return backbone::select(std::forward<MASK>(mask), std::forward<A>(a), std::forward<B>(b));
    }
  } // end detail namespace

  //! @{ \name select generalization to (named) tuples and types with reflection support
  template<typename MASK, typename A> requires (concepts::reflection::supported<A> || is_tuple_v<anonymize_t<A>>) 
    inline constexpr auto select(MASK&& mask, const A& a, const A& b)   // Allow for passing initializer list as first or second argument for types with reflection or tuples (others should be handled by backbone::select)
  {
    return bbm::detail::select(std::forward<MASK>(mask), a, b);
  }

  template<typename MASK, typename A, typename B> requires requires(MASK m, A a, B b) {{bbm::detail::select(m,a,b)};}
    inline constexpr auto select(MASK&& mask, A&& a, B&& b)             // General case; heterogeneous arguments
  {
    return bbm::detail::select(std::forward<MASK>(mask), std::forward<A>(a), std::forward<B>(b));
  }
  //! @}

  /**********************************************************************/
  /*! \brief Generalize backbone::lookup to include containers of tuples/named
      tuples/reflection-supported types

    \tparam RET = return type; must support reflection
    \param container = a container to lookup from
    \param idx = index of the elements to lookup
    \param mask = enable/disable lanes

  When tuple/named or reflection supported type, we lookup each element
  seperately to support packet data structures.
    
  **********************************************************************/
  template<typename RET, typename C, typename Index> requires
  std::ranges::range<C> &&                                            // C must be a range
  ((std::constructible_from<RET> &&                                    // (trivially constructible AND
   ((is_tuple_v<RET> && is_tuple_v<std::ranges::range_value_t<C>>) || // (tuple OR
    (named_equivalence_v<RET, std::ranges::range_value_t<C>>) ||      // named OR
    (concepts::reflection::supported<RET> && concepts::reflection::supported<std::ranges::range_value_t<C>>))) || // unnamed)) OR
   requires(C&& c, Index i, index_mask_t<Index> m) {{backbone::lookup<RET>(c,i,m)};})      // has backbone support
    inline RET lookup(C&& container, const Index& idx, const index_mask_t<Index>& mask=true)
  {
    using Value = std::ranges::range_value_t<std::decay_t<C>>;

    // Case 1: tuples
    if constexpr (std::constructible_from<RET> && is_tuple_v<RET> && is_tuple_v<Value>)
    {
      static_assert(std::tuple_size_v<std::decay_t<RET>> == std::tuple_size_v<std::decay_t<Value>>, BBM_SIZE_MISMATCH);

      RET result;
      auto lookup_helper = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        ((std::get<IDX>(result) = bbm::lookup< std::decay_t<decltype(std::get<IDX>(result))> >(  std::ranges::views::transform(container, [](auto&& v) { return std::get<IDX>(std::forward<decltype(v)>(v)); }), idx, mask )), ...);
      };

      lookup_helper(std::make_index_sequence<std::tuple_size_v<std::decay_t<RET>>>{});
      return result;
    }

    // Case 2: named
    else if constexpr (std::constructible_from<RET> && named_equivalence_v<RET, Value>)
    {
      RET result;
      auto lookup_helper = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        ((bbm::get<std::decay_t<RET>::template name<IDX>>(result) = bbm::lookup< std::decay_t<decltype(bbm::get<std::decay_t<RET>::template name<IDX>>(result))> >( std::ranges::views::transform(container, [](auto&& v) { return bbm::get<std::decay_t<RET>::template name<IDX>>(std::forward<decltype(v)>(v)); }), idx, mask)), ... );
      };

      lookup_helper(std::make_index_sequence<std::tuple_size_v<std::decay_t<RET>>>{});
      return result;
    }

    // Case 3: reflection support
    else if constexpr (std::constructible_from<RET> && concepts::reflection::supported<RET> && concepts::reflection::supported<Value>)
    {
      static_assert(reflection::attributes_size<RET> == reflection::attributes_size<std::ranges::range_value_t<C>>, BBM_SIZE_MISMATCH);

      RET result;
      auto lookup_helper = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        ((std::get<IDX>(reflection::attributes(result)) = bbm::lookup<std::decay_t<std::tuple_element_t<IDX,reflection::attributes_t<RET>>>>( std::ranges::views::transform(container, [](auto&& v) { return std::get<IDX>(reflection::attributes(std::forward<decltype(v)>(v))); }), idx, mask )), ...);
      };

      lookup_helper(std::make_index_sequence<reflection::attributes_size<RET>>{});
      return result;
    }
    
    // Otherwise forward to backbone
    else return backbone::lookup<RET>(std::forward<C>(container), idx, mask);
  }
    
  /**********************************************************************/
  /*! \brief Generalization of backbone::set to include
      tuples/named/reflection-supported objects.

      \param container: container to set data in
      \param indx: index at which to set the data
      \param value: value to store
      \param mask: enable/disbale lanes

  When tuple/named or reflection-type, we set each element seperately to
  support packet data structures.
    
  ***********************************************************************/
  template<typename Value, typename C, typename Index>
  inline void set(C&& container, const Index& idx, Value&& value, const index_mask_t<Index>& mask=true)
  {
    using CValue = std::ranges::range_value_t<C>;

    // Case 1: tuple
    if constexpr (is_tuple_v<Value> && is_tuple_v<CValue>)
    {
      static_assert(std::tuple_size_v<std::decay_t<Value>> == std::tuple_size_v<std::decay_t<CValue>>, BBM_SIZE_MISMATCH);

      auto set_helper = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        (bbm::set(std::ranges::views::transform(std::forward<C>(container), [](auto&& v) -> decltype(auto) { return std::get<IDX>(std::forward<decltype(v)>(v)); }), idx, std::get<IDX>(std::forward<Value>(value)), mask ), ...);
      };

      set_helper(std::make_index_sequence<std::tuple_size_v<std::decay_t<CValue>>>{});
    }

    // Case 2: named
    if constexpr (named_equivalence_v<Value, CValue>)
    {
      auto set_helper = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        (bbm::set(std::ranges::views::transform(std::forward<C>(container), [](auto&& v) -> decltype(auto) { return bbm::get<std::decay_t<CValue>::template name<IDX>>(std::forward<decltype(v)>(v)); }), idx, bbm::get<std::decay_t<CValue>::template name<IDX>>(std::forward<Value>(value)), mask ), ...);
      };

      set_helper(std::make_index_sequence<std::tuple_size_v<std::decay_t<CValue>>>{});
    }

    // Case 3: reflection supported
    if constexpr (concepts::reflection::supported<Value> && concepts::reflection::supported<CValue>)
    {
      auto set_helper = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        (bbm::set(std::ranges::views::transform(std::forward<C>(container), [](auto&& v) -> decltype(auto) { return std::get<IDX>(reflection::attributes(std::forward<decltype(v)>(v))); }), idx, std::get<IDX>(reflection::attributes(std::forward<Value>(value))), mask ), ...);
      };

      set_helper(std::make_index_sequence<reflection::attributes_size<CValue>>{});

    }
    
    // Otherwise forward to backbone
    else backbone::set(std::forward<C>(container), idx, std::forward<Value>(value), mask);
  }
  
  //! @{ \name Gradient methods

  /**********************************************************************/
  /*! \brief Detach the value from the gradient computations

    \param t = variable to detach
    \returns a reference to the raw value

    Note: if the variable does not support gradients, then the value is
    returned unchanged.
  ***********************************************************************/
  template<typename T>
    auto detach_gradient(T& t)
  {
    if constexpr (is_diff_v<T>) { return backbone::detach_gradient(t); }
    else { return t; }
  }

  /**********************************************************************/
  /*! \brief Return the gradient

    \param t = variable to return the tracked gradient
    \returns tracked gradient (const reference)

    static_assert if T is not differentiable.
  ***********************************************************************/
  template<typename T>
    auto gradient(T& t)
  {
    static_assert(is_diff_v<T>, BBM_NOT_DIFF);
    return backbone::gradient(t);
  }

  /**********************************************************************/
  /*! \brief Checks if gradients are enabled for a variable

    \param t = variable to check
    \returns true if the variable is tracked.
  ***********************************************************************/
  template<typename T>
    bool is_gradient_tracked(const T& t)
  {
    if constexpr (is_diff_v<T>) { return backbone::is_gradient_tracked(t); }
    else return false;
  }
  
  /**********************************************************************/
  /*! \brief Enable/disable tracking of gradients for a variable

    \param t = variable to track
    \param toggle = enable/disable

    static_assert if T is not differentiable.
  ***********************************************************************/
  template<typename T>
    void track_gradient(T& t, bool toggle=true)
  {
    static_assert(is_diff_v<T>, BBM_NOT_DIFF);
    backbone::track_gradient(t, toggle);
  }

  /**********************************************************************/
  /*! \brief Enable forward gradient tracking

    \param t = variable to track

    static_assert if T is not differentiable.
  ***********************************************************************/
  template<typename T>
    void forward_gradient(T& t)
  {
    static_assert(is_diff_v<T>, BBM_NOT_DIFF);
    backbone::forward_gradient(t);
  }

  /**********************************************************************/
  /*! \brief Enable backward/reverse gradient tracking

    \param t = variable to track

    static_assert if T is not differentiable.
  ***********************************************************************/
  template<typename T>
    void backward_gradient(T& t)
  {
    static_assert(is_diff_v<T>, BBM_NOT_DIFF);
    backbone::backward_gradient(t);
  }
  //! @}
  
} // end bbm namespace


/************************************************************************/
/* Include backbone concepts after the declaration of the bbm namespace
   since the concepts assume the methods and typedefs are declared
*************************************************************************/
#include "concepts/macro.h"
#include "concepts/backbone/type_traits.h"
#include "concepts/backbone/ordered.h"
#include "concepts/backbone/math.h"
#include "concepts/backbone/horizontal.h"
#include "concepts/backbone/control.h"
#include "concepts/backbone/gradient.h"
#include "concepts/backbone/complex.h"
#include "concepts/backbone/random.h"

#endif /* _BBM_BACKBONE_H_ */
