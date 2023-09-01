#ifndef _BBM_DRJIT_CONTROL_H_
#define _BBM_DRJIT_CONTROL_H_

#include "core/error.h"
#include "util/iterator_util.h"
#include "backbone/type_traits.h"
#include "drjit/util.h"

/************************************************************************/
/*! \file control.h

  \brief Data and flow control
  + select
  + lookup
  + cast
  + set
  + binary_search
  
*************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! \brief cast
   **********************************************************************/
  template<typename NEWTYPE, typename OLDTYPE>
    NEWTYPE cast(OLDTYPE&& val)
  {
    if constexpr (is_diff_v<OLDTYPE> && !is_diff_v<NEWTYPE>) return cast<NEWTYPE>(drjit::detach(val));
    else if constexpr (is_LLVMArray_v<OLDTYPE> && !is_LLVMArray_v<NEWTYPE>) return cast<NEWTYPE>(val[0]);
    else if constexpr (drjit::is_static_array_v<OLDTYPE> && drjit::is_static_array_v<NEWTYPE>)
    {
      auto helper = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        return NEWTYPE( cast<value_t<NEWTYPE>>( val[IDX] )... );
      };

      return helper(std::make_index_sequence<std::decay_t<OLDTYPE>::Size>{});
    }
    else return NEWTYPE(val);
  }
    
  /**********************************************************************/
  /*! \brief Extension of drjit::select to diff/non-diff masks
    *********************************************************************/
  template<typename MASK, typename A, typename B>
    inline auto select(MASK&& mask, A&& a, B&& b)
  {
    if constexpr (is_diff_v<MASK> && !is_diff_v<A> && !is_diff_v<B>)
      return drjit::select(cast<remove_diff_t<MASK>>(mask), std::forward<A>(a), std::forward<B>(b));
    else if constexpr (is_LLVMArray_v<MASK>  && !is_LLVMArray_v<A> && !is_LLVMArray_v<B>)
      return drjit::select(mask[0], std::forward<A>(a), std::forward<B>(b));
    else
      return drjit::select(std::forward<MASK>(mask), std::forward<A>(a), std::forward<B>(b));
  }

  /**********************************************************************/
  /*! \brief Non-packet look up
   **********************************************************************/
  template<typename RET, typename C, typename Index> requires (!is_packet_v<Index>) && std::ranges::range<C> && std::convertible_to<bbm::iterable_value_t<C>, RET> && is_index_v<Index>
    inline constexpr RET lookup(C&& container, const Index& idx, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return RET();

    // lookup
    size_t i = cast<size_t>(idx);
    if(i >= bbm::size(container)) throw bbm_out_of_range;
    return RET( *(std::next(bbm::begin(container), i)));
  }
  
  
  /**********************************************************************/
  /*! \brief Non-packet data, Packet look up
   **********************************************************************
  template<typename RET, typename C, typename Index> requires is_packet_v<RET> && is_packet_v<Index> && std::ranges::range<C> && is_index_v<Index> && std::convertible_to<bbm::iterable_value_t<C>, remove_packet_t<RET>> && (!is_packet_v<bbm::iterable_value_t<C>>)
    inline RET lookup(C&& container, const Index& idx, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return RET();

    // lookup
    RET result;

    static_assert(bbm::dependent_true_v<RET>, "NOT IMPLEMENTED");
    for(size_t i=0; i != idx.Size; ++i)
      if(mask[i])
        result[i] = lookup<bbm::iterable_value_t<C>>(std::forward<C>(container), idx[i]);
        
    // Done.
    return result;
  }
  */
  
  /**********************************************************************/
  /*! \brief Packet data, Packet look up
    *********************************************************************
  template<typename RET, typename C, typename Index> requires is_packet_v<RET> && is_packet_v<Index> && std::ranges::range<C> && is_index_v<Index> && std::convertible_to<bbm::iterable_value_t<C>, RET> && is_packet_v<bbm::iterable_value_t<C>>
    inline RET lookup(C&& container, const Index& idx, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return RET();
    
    // lookup
    RET result;

    static_assert(bbm::dependent_true_v<RET>, "NOT IMPLEMENTED");
    for(size_t i=0; i != idx.Size; ++i)
      if( mask[i] )
      {
        if(drjit::slice(idx, i) >= bbm::size(container)) throw bbm_out_of_range;
        else drjit::slice(result, i) = drjit::slice(*(std::next(bbm::begin(container), drjit::slice(idx, i))), i); 
      }
    
    // Done.
    return result;
  }
  */

  /**********************************************************************/
  /*! \brief Non-packet set
   **********************************************************************/
  template<typename VAL, typename C, typename Index> requires (!is_packet_v<Index>) && std::ranges::range<C> && is_index_v<Index> && std::convertible_to<VAL, bbm::iterable_value_t<C>>
  inline constexpr void set(C&& container, const Index& idx, VAL&& value, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return;

    // set
    size_t i = cast<size_t>(idx);
    if(i >= bbm::size(container)) throw bbm_out_of_range;
    *(std::next(bbm::begin(std::forward<C>(container)), i)) = std::forward<VAL>(value);
  }
  

  /**********************************************************************/
  /*! \brief Non-packet data, Packet set
   **********************************************************************
  template<typename VAL, typename C, typename Index> requires is_packet_v<VAL> && is_packet_v<Index> && std::ranges::range<C> &&  is_index_v<Index> && std::convertible_to<remove_packet_t<VAL>, bbm::iterable_value_t<C>> && (!is_packet_v<bbm::iterable_value_t<C>>)
    inline void set(C&& container, const Index& idx, VAL&& value, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return;

    static_assert(bbm::dependent_true_v<VAL>, "NOT IMPLEMENTED");
    // set
    for(size_t i=0; i != idx.Size; ++i)
      set(std::forward<C>(container), drjit::slice(idx, i), drjit::slice(value, i), drjit::slice(mask, i));
  }
  */

  /**********************************************************************/
  /*! \brief Packet data, Packet set
    *********************************************************************
  template<typename VAL, typename C, typename Index> requires is_packet_v<VAL> && is_packet_v<Index> && std::ranges::range<C> && is_index_v<Index> && std::convertible_to<VAL, bbm::iterable_value_t<C>> && is_packet_v<bbm::iterable_value_t<C>>
    inline void set(C&& container, const Index& idx, VAL&& value, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return;

    static_assert(bbm::dependent_true_v<VAL>, "NOT IMPLEMENTED");
    // set
    for(size_t i=0; i != idx.Size; ++i)
      if( drjit::slice(mask, i) )
      {
        if(drjit::slice(idx, i) >= bbm::size(container)) throw bbm_out_of_range;
        drjit::slice(*(std::next(bbm::begin(container), drjit::slice(idx, i))), i) = drjit::slice(value, i); 
      }
  }
  */
  
  /**********************************************************************/
  /*! \brief binary search
   **********************************************************************/
  template<typename C, typename PRED> requires std::ranges::range<C> && std::is_invocable_r_v<mask_t<bbm::iterable_value_t<C>>, PRED, bbm::iterable_value_t<C>>
  inline constexpr index_t<bbm::iterable_value_t<C>> binary_search(C&& container, PRED&& predicate, const index_mask_t<bbm::iterable_value_t<C>>& mask=true)
  {
    using value_type = bbm::iterable_value_t<C>;
    using index_type = index_t<value_type>;
    
    // quick exit
    if(none(mask)) return index_type(bbm::size(container));
    
    // create a wrapper for the predicate to meet drjit's expectations
    auto pred_wrapper = [&](const index_type& index)
    {
      auto result = predicate( lookup<value_type>(container, index, mask) );
      return cast<index_mask_t<index_type>>(result);
    };

    // pass control to drjit
    index_type idx = drjit::binary_search<index_type>(0, bbm::size(container)-1, pred_wrapper);
    return select(mask && !pred_wrapper(idx), idx, bbm::size(container));
  }
  
} // end backbone namespace


#endif /* _BBM_DRJIT_CONTROL_H_ */
