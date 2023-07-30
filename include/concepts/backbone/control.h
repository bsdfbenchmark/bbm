#ifndef _BBM_CONTROL_BACKBONE_CONCEPT_H_
#define _BBM_CONTROL_BACKBONE_CONCEPT_H_

#include <vector>
#include "concepts/util.h"

/************************************************************************/
/*! \file control.h

  \brief Flow control, lookup and cast operations.
  
*************************************************************************/

namespace bbm {
  namespace concepts {
    namespace backbone {

      /************************************************************************/
      /*! \brief A type T has valid control methods if:

        + mask_t<T> is defined
        + value_t<T> is defined
        + remove_diff_t<T> is defined
        + index_t<T> is defined
        + cast<NewType>(oldType) cast oldType to newType
        + select(Mask, T, T) returns a type T based on the mask.
        + lookup<T>(container, index_t, index_mask_t) returns a type T; testing on std::vector as container
        + set(container, index_t, T, index_mask_t) set a value in a container taking; similar to lookup handles packet data
        + binary_search(container, predicate, index_mask_t) returns the index of the first element in container for which predicate is false.
  
*************************************************************************/
      template<typename T>
        concept control = requires(T a)
      {
        requires concepts::valid_trait<mask_t, T>;
        requires concepts::valid_trait<value_t, T>;
        requires concepts::valid_trait<remove_diff_t, T>;
        requires concepts::valid_trait<remove_packet_t, T>;
        requires concepts::valid_trait<index_t, T>;

        /****************************************************************/
        /*! \brief cast<NEWTYPE>(T) between a type T and NEWTYPE; test with NEWTYPE == T
          ***************************************************************/
        { cast<T>( std::declval<T>() ) } -> std::same_as<T>;

        /****************************************************************/
        /*! \brief select(mask_t<T>, T, T) between two options based on a mask
          
          Given a mask (possible a packet), return the first element if
          the mask is true, otherwise return the second value.  If the mask and
          the options are packets, return a packet where each element matches
          the choice determined by the corresponding mask
        *****************************************************************/
        { select( std::declval<mask_t<T>>(), a, a ) } -> std::convertible_to<T>;


        /****************************************************************/
        /*! \brief lookup<RET>(container, idx, mask=true)

          \tparam RET = return type
          \param container = iterable container
          \param idx = index to gather from container either index_t<T> or size_t
          \param mask = enable/disbale; must be index_mask_t<decltype(idx)>

          Must support the following four cases:
          
          Case 0: RET!=packet, Value(C)!=packet, Index!=packet
          Case 1: RET==packet, Value(C)==packet, Index!=packet
          Case 2: RET==packet, Value(C)!=packet, Index==packet
          Case 3: RET==packet, Value(C)==packet, Index==packet

          Case 0 and 1 are equal to a regular lookup: container[idx] & mask.
          Case 2 is equivallent to: RET[i] = C[idx[i]] & mask[i]
          Case 3 is equivallent to: RET[i] = C[idx[i]][i] & mask[i]

          Thus either: (Value(C) == packet) == (RET == Packet) or (RET ==
          Packet) == (Index == Packet).

          Check concept for std::vector as container and all combinations for
          which RET = T
        *****************************************************************/
        { lookup<T>( std::declval<std::vector<remove_packet_t<T>>>(), std::declval<size_t>(), std::declval<index_mask_t<size_t>>() ) } -> std::convertible_to<T>;  // Case 0
        { lookup<T>( std::declval<std::vector<T>>(), std::declval<size_t>(), std::declval<index_mask_t<size_t>>() ) } -> std::convertible_to<T>;                   // Case 1
        { lookup<T>( std::declval<std::vector<remove_packet_t<T>>>(), std::declval<index_t<T>>(), std::declval<index_mask_t<T>>() ) } -> std::convertible_to<T>;   // Case 2
        { lookup<T>( std::declval<std::vector<T>>(), std::declval<index_t<T>>(), std::declval<index_mask_t<T>>() ) } -> std::convertible_to<T>;                    // Case 3
        
        { lookup<T>( std::declval<std::vector<remove_packet_t<T>>>(), std::declval<size_t>() ) } -> std::convertible_to<T>;       // Case 0; default mask
        { lookup<T>( std::declval<std::vector<T>>(), std::declval<size_t>()) } -> std::convertible_to<T>;                         // Case 1; default mask
        { lookup<T>( std::declval<std::vector<remove_packet_t<T>>>(), std::declval<index_t<T>>() ) } -> std::convertible_to<T>;   // Case 2; default mask
        { lookup<T>( std::declval<std::vector<T>>(), std::declval<index_t<T>>() ) } -> std::convertible_to<T>;                    // Case 3; default mask

        /****************************************************************/
        /*! \brief set(container, idx, value, mask=true)

          \param container = iterable container
          \param idx = index to set; either index_t<T> or size_t
          \param value = value to set
          \param mask = enable/disable; must be index_mask_t<decltype(idx)>

          Must supprt the following four cases:

          Case 0: VAL!=packet, Value(C)!=packet, Index!=packet
          Case 1: VAL==packet, Value(C)==packet, Index!=packet
          Case 2: VAL==packet, Value(C)!=packet, Index==packet
          Case 3: VAL==packet, Value(C)==packet, Index==packet

          Case 0 and 1 are equivalent to container[idx] = value
          Case 2 is equivallent to if(mask[i]) C[idx[i]] = value[i]
          Case 3 is equivallent to if(mask[i]) C[idx[i]][i] = value[i]

          Check concept for std::vector as container for all combinations with VAL=T
        *****************************************************************/
        { set( std::declval<std::vector<remove_packet_t<T>>>(), std::declval<size_t>(), std::declval<remove_packet_t<T>>(), std::declval<index_mask_t<size_t>>() ) }; // Case 0
        { set( std::declval<std::vector<T>>(), std::declval<size_t>(), std::declval<T>(), std::declval<index_mask_t<size_t>>() ) };                                   // Case 1
        { set( std::declval<std::vector<remove_packet_t<T>>>(), std::declval<index_t<T>>(), std::declval<T>(), std::declval<index_mask_t<T>>() ) };                   // Case 2
        { set( std::declval<std::vector<T>>(), std::declval<index_t<T>>(), std::declval<T>(), std::declval<index_mask_t<T>>() ) };                                    // Case 3

        { set( std::declval<std::vector<remove_packet_t<T>>>(), std::declval<size_t>(), std::declval<remove_packet_t<T>>() ) }; // Case 0; default mask
        { set( std::declval<std::vector<T>>(), std::declval<size_t>(), std::declval<T>() ) };                                   // Case 1; default mask
        { set( std::declval<std::vector<remove_packet_t<T>>>(), std::declval<index_t<T>>(), std::declval<T>() ) };              // Case 2; default mask
        { set( std::declval<std::vector<T>>(), std::declval<index_t<T>>(), std::declval<T>() ) };                               // Case 3; default mask
        
        /****************************************************************/
        /*! \brief binary_search(containr, predicate, mask=true)

          \param container = iterable container of data to search

          \param predicate = a function that takes a data element from the
                             contrainer and returns a mask indicating whether
                             if meets or does not meet the condition.
          \param mask = enable/disable lanes
          \returns the index of first element (per lane) for which the
                   predicate is false. Return container.size() if all are
                   true.

          Check concept for std::vector<T> and a placeholder predicate
          ***************************************************************/
        { binary_search( std::declval<std::vector<T>>(), [](const T&) { return true; }, std::declval<index_mask_t<T>>() ) } -> std::convertible_to< index_t<T> >;
        { binary_search( std::declval<std::vector<T>>(), [](const T&) { return true; } ) } -> std::convertible_to< index_t<T> >;
        
      };

      
    } // end backbone namespace
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_CONTROL_BACKBONE_CONCEPT_H_ */
    
