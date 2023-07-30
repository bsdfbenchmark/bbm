#ifndef _BBM_TYPETRAITS_BACKBONE_CONCEPT_H_
#define _BBM_TYPETRAITS_BACKBONE_CONCEPT_H_

#include "concepts/util.h"

/************************************************************************/
/*! \file type_traits.h

  \brief A valid backbone type must have the following type_traits:
  + is_diff_v<T>: true if T is differentiable
  + remove_diff_t<T>: returns T without differentiability
  + is_packet_v<T>: true if T is a packet
  + remove_packet_t<T>: returns T without packet
  + mask_t<T>: returns the mask type corresponding to T
  + is_mask_v<T>: returns true if T is a mask
  + value_t<T>: returns the value type
  + index_t<T>: return the index (size_t) type corresponding to T.

**************************************************************************/

namespace bbm {
  namespace concepts {
    namespace backbone {

      template<typename T>
        concept type_traits = requires
      {
        requires concepts::valid_trait< remove_diff_t, T >;
        requires concepts::valid_trait< add_diff_t, T >;
        requires concepts::valid_trait< remove_packet_t, T >;
        requires concepts::valid_trait< add_packet_t, T >;
        requires concepts::valid_trait< mask_t, T >;
        requires concepts::valid_trait< index_t, T >;
        requires concepts::valid_trait< index_mask_t, T >;
        requires concepts::valid_trait< value_t, T >;
        requires concepts::valid_trait< scalar_t, T >;
        requires concepts::valid_trait< replace_scalar_t, T, size_t >;
        
        { is_diff_v<T> } -> std::convertible_to<bool>;
        { is_packet_v<T> } -> std::convertible_to<bool>;
        { is_mask_v<T> } -> std::convertible_to<bool>;
        { is_index_v<T> } -> std::convertible_to<bool>;

        requires is_mask_v<mask_t<T>>;
        requires is_index_v<index_t<T>>;
      };

    } // end backbone namespace
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_TYPETRAITS_BACKBONE_CONCEPT_H_ */
