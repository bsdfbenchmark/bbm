#ifndef _BBM_ENOKI_CONTROL_H_
#define _BBM_ENOKI_CONTROL_H_

#include "core/error.h"
#include "util/iterator_util.h"
#include "backbone/type_traits.h"
#include "backbone/array.h"

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
    if constexpr (is_diff_v<OLDTYPE> && !is_diff_v<NEWTYPE>) return NEWTYPE(enoki::detach(val));
    else return NEWTYPE(val);
  }
    
  /**********************************************************************/
  /*! \brief Extension of enoki::select robust to diff/non-diff masks
    *********************************************************************/
  template<typename MASK, typename A, typename B>
    inline auto select(MASK&& mask, A&& a, B&& b)
  {
    if constexpr (std::same_as<std::decay_t<MASK>, detail::bbm_mask>)
      return backbone::select(enoki::DiffArray<bool>(mask), std::forward<A>(a), std::forward<B>(b));
    else if constexpr (is_diff_v<MASK> && !is_diff_v<A> && !is_diff_v<B>)
      return enoki::select(enoki::detach(mask), std::forward<A>(a), std::forward<B>(b));
    else if constexpr (!is_diff_v<MASK> && is_diff_v<A>)
      return enoki::select(mask_t<A>(mask), std::forward<A>(a), std::forward<B>(b));
    else if constexpr (!is_diff_v<MASK> && is_diff_v<B>)
      return enoki::select(mask_t<B>(mask), std::forward<A>(a), std::forward<B>(b));
    else if constexpr (is_packet_v<MASK> && !is_packet_v<A> && !is_packet_v<B>)
      return enoki::select(std::forward<MASK>(mask), enoki::Packet<A>(a), enoki::Packet<B>(b));
    else
      return enoki::select(std::forward<MASK>(mask), std::forward<A>(a), std::forward<B>(b));
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
    if(idx >= Index(bbm::size(container))) throw bbm_out_of_range;
    return RET( *(std::next(bbm::begin(container), idx)));
  }
  
  
  /**********************************************************************/
  /*! \brief Non-packet data, Packet look up
   **********************************************************************/
  template<typename RET, typename C, typename Index> requires is_packet_v<RET> && is_packet_v<Index> && std::ranges::range<C> && is_index_v<Index> && std::convertible_to<bbm::iterable_value_t<C>, remove_packet_t<RET>> && (!is_packet_v<bbm::iterable_value_t<C>>)
    inline RET lookup(C&& container, const Index& idx, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return RET();

    // lookup
    RET result;
    for(size_t i=0; i != idx.Size; ++i)
      if(enoki::slice(mask,i))
        enoki::packet(result, i) = backbone::lookup<bbm::iterable_value_t<C>>(std::forward<C>(container), enoki::slice(idx, i));
      
    // Done.
    return result;
  }

  /**********************************************************************/
  /*! \brief Packet data, Packet look up
    *********************************************************************/
  template<typename RET, typename C, typename Index> requires is_packet_v<RET> && is_packet_v<Index> && std::ranges::range<C> && is_index_v<Index> && std::convertible_to<bbm::iterable_value_t<C>, RET> && is_packet_v<bbm::iterable_value_t<C>>
    inline RET lookup(C&& container, const Index& idx, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return RET();
    
    // lookup
    RET result;
    for(size_t i=0; i != idx.Size; ++i)
      if( enoki::slice(mask, i) )
      {
        if(enoki::slice(idx, i) >= bbm::size(container)) throw bbm_out_of_range;
        else enoki::slice(result, i) = enoki::slice(*(std::next(bbm::begin(container), enoki::slice(idx, i))), i); 
      }
    
    // Done.
    return result;
  }


  /**********************************************************************/
  /*! \brief Non-packet set
   **********************************************************************/
  template<typename VAL, typename C, typename Index> requires (!is_packet_v<Index>) && std::ranges::range<C> && is_index_v<Index> && std::convertible_to<VAL, bbm::iterable_value_t<C>>
  inline constexpr void set(C&& container, const Index& idx, VAL&& value, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return;

    // set
    if(idx >= Index(bbm::size(container))) throw bbm_out_of_range;
    *(std::next(bbm::begin(std::forward<C>(container)), idx)) = std::forward<VAL>(value);
  }
  

  /**********************************************************************/
  /*! \brief Non-packet data, Packet set
   **********************************************************************/
  template<typename VAL, typename C, typename Index> requires is_packet_v<VAL> && is_packet_v<Index> && std::ranges::range<C> &&  is_index_v<Index> && std::convertible_to<remove_packet_t<VAL>, bbm::iterable_value_t<C>> && (!is_packet_v<bbm::iterable_value_t<C>>)
    inline void set(C&& container, const Index& idx, VAL&& value, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return;

    // set
    for(size_t i=0; i != idx.Size; ++i)
      backbone::set(std::forward<C>(container), enoki::slice(idx, i), enoki::slice(value, i), enoki::slice(mask, i));
  }


  /**********************************************************************/
  /*! \brief Packet data, Packet set
    *********************************************************************/
  template<typename VAL, typename C, typename Index> requires is_packet_v<VAL> && is_packet_v<Index> && std::ranges::range<C> && is_index_v<Index> && std::convertible_to<VAL, bbm::iterable_value_t<C>> && is_packet_v<bbm::iterable_value_t<C>>
    inline void set(C&& container, const Index& idx, VAL&& value, const index_mask_t<Index>& mask=true)
  {
    // quick bailout
    if(none(mask)) return;
    
    // set
    for(size_t i=0; i != idx.Size; ++i)
      if( enoki::slice(mask, i) )
      {
        if(enoki::slice(idx, i) >= bbm::size(container)) throw bbm_out_of_range;
        enoki::slice(*(std::next(bbm::begin(container), enoki::slice(idx, i))), i) = enoki::slice(value, i); 
      }
  }

  
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
    
    // create a wrapper for the predicate to meet enoki's expectations
    auto pred_wrapper = [&](const index_type& index)
    {
      auto result = predicate( backbone::lookup<value_type>(container, index, mask) );
      return cast<index_mask_t<index_type>>(result);
    };

    // pass control to enoki
    index_type idx = enoki::binary_search(0, bbm::size(container)-1, pred_wrapper);
    return select(mask && !pred_wrapper(idx), idx, bbm::size(container));
  }
  
} // end backbone namespace


#endif /* _BBM_ENOKI_CONTROL_H_ */
