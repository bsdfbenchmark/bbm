#ifndef _BBM_INOUT_LINEARIZER_CONCEPT_H_
#define _BBM_INOUT_LINEARIZER_CONCEPT_H_

#include "bbm/config.h"

/************************************************************************/
/*! \file inout_linearizer.h

  \brief inout_linearizer contract
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief inout_linearizer concept

      An inout_linearizer enumerates a discrete set of (in,out) directions
      on the joint incident and outgoing direction sphere.
      
      Each inout_linearizer contains the following:
      + concept::has_config
      + Size_t size(void) const:  the number of discrete direction-pairs
      + Vec3dPair opeator()(Size_t index, Mask mask=true) const: returns the index-th direction pair
      + Size_t operator()(const Vec3d& in, const Vec3d& out, Mask mask=true) const: the inverse operation
    *********************************************************************/
    template<typename T>
      concept inout_linearizer = requires(const T& t)
    {
      requires concepts::has_config<T>;
      { t.size() } -> std::same_as<Size_t<T>>;
      
      { t.operator()(std::declval<Size_t<T>>(), std::declval<Mask_t<T>>()) } -> std::same_as<Vec3dPair_t<T>>;
      { t.operator()(std::declval<Size_t<T>>()) } -> std::same_as<Vec3dPair_t<T>>;

      { t.operator()(std::declval<Vec3d_t<T>>(), std::declval<Vec3d_t<T>>(), std::declval<Mask_t<T>>()) } -> std::same_as<Size_t<T>>;
      { t.operator()(std::declval<Vec3d_t<T>>(), std::declval<Vec3d_t<T>>()) } -> std::same_as<Size_t<T>>;
    };

    /********************************************************************/
    /*! \brioef inout_linearizer archetype for concept checking

      \tparam CONF = config to check for. Default = archetype::config
    *********************************************************************/
    namespace archetype {
      template<typename CONF=config> requires concepts::config<CONF>
        struct inout_linearizer
      {
        using Config = CONF;
        Size_t<Config> size(void) const;
        Vec3dPair_t<Config> operator()(const Size_t<Config>&, Mask_t<Config> = true) const;
        Size_t<Config> operator()(const Vec3d_t<Config>&, const Vec3d_t<Config>&, Mask_t<Config> = true) const;
      };
    } // end archetype namespace

    BBM_CHECK_CONCEPT(concepts::inout_linearizer, archetype::inout_linearizer<>);
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_INOUT_LINEARIZER_CONCEPT_H_ */
