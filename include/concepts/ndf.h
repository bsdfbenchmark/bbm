#ifndef _BBM_NDF_CONCEPT_H_
#define _BBM_NDF_CONCEPT_H_

#include "concepts/config.h"
#include "concepts/named.h"
#include "concepts/util.h"

/************************************************************************/
/*! \file ndf.h

  \brief Microfacet Normal Distribution contract
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief ndf concept

      Each ndf requires:
      + concepts::config
      + concepts::named
      + Value/Spectrum eval(const Vec3d& halfway, Mask mask=true) const
      + Vec3d sample(const Vec3d& view, const Vec2d& xi, Mask mask=true) const
      + Value pdf(const Vec3d& view, const Vec3d& m, Mask mask=true) const
      + Value/Spectrum G1(const Vec3d& v, const Vec3d& m, Mask mask=true) const
    *********************************************************************/
    template<typename NDF>
      concept ndf = requires(const NDF& ndf)
    {
      requires concepts::config<NDF>;
      requires concepts::named<NDF>;
      
      /******************************************************************/
      /*! \brief Evaluate the NDF given an in and out direction

        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        Value/Spectrum eval(const Vec3d& halfway, Mask mask=true) const
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        \param halfway = hafway vector
        \param mask = mask to enable/disable lanes [default = true]
        \returns the resulting Value (or Spectrum) of the evaluation.
      *******************************************************************/
      { ndf.eval(std::declval<Vec3d_t<NDF>>(), std::declval<Mask_t<NDF>>()) } -> concepts::same_as_any<Value_t<NDF>, Spectrum_t<NDF>>;

      //! \brief mask has default value
      { ndf.eval(std::declval<Vec3d_t<NDF>>()) } -> concepts::same_as_any<Value_t<NDF>, Spectrum_t<NDF>>;

        
      /******************************************************************/
      /*! \brief Samples a microfacet normal with a pdf proportional to the NDF

        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        Vec3d sample(const Vec3d& view, const Vec2d& xi, Mask mask=true) const
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        \param view = view direction (to determine visible normals)
        \param xi = two random variables stored in a Vec2d used to sample
        \param mask = mask to enable/disable lanes [default=true]
        \returns a Vec3d with the sampled microfacet normal vector.
      *******************************************************************/
      { ndf.sample(std::declval<Vec3d_t<NDF>>(), std::declval<Vec2d_t<NDF>>(), std::declval<Mask_t<NDF>>()) } -> std::same_as<Vec3d_t<NDF>>;

      //! \brief mask has default value
      { ndf.sample(std::declval<Vec3d_t<NDF>>(), std::declval<Vec2d_t<NDF>>()) } -> std::same_as<Vec3d_t<NDF>>;

      /******************************************************************/
      /*! \brief The pdf of the sampled microfacet normal.

        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        Value pdf(const Vec3d& view, const Vec3d& m, Mask mask=true) const
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        \param view = view direction (to determine visible normals)
        \param m = microfacet normal
        \param mask = mask to enable/disable lanes [default=true]
        \returns the pdf (Value) of sampling the halfway vector
      *******************************************************************/
      { ndf.pdf(std::declval<Vec3d_t<NDF>>(), std::declval<Vec3d_t<NDF>>(), std::declval<Mask_t<NDF>>()) } -> std::same_as<Value_t<NDF>>;

      //! \brief has default mask value
      { ndf.pdf(std::declval<Vec3d_t<NDF>>(), std::declval<Vec3d_t<NDF>>()) } -> std::same_as<Value_t<NDF>>;

      /******************************************************************/
      /*! \brief The unidirectional geometrical attenuation of shadowing and masking

        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        Value/Spectrum G1(const Vec3d in, const Vec3d& out, const Vec3d& m, Mask mask=true) const
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        \param v = incident/outgoing direction of transport
        \param m = microfacet normal
        \param mask = mask to enable/disable lanes [default=true]
        \returns the geometrical attenuation factor (Value or Spectrum).
      *******************************************************************/
      { ndf.G1(std::declval<Vec3d_t<NDF>>(), std::declval<Vec3d_t<NDF>>(), std::declval<Mask_t<NDF>>()) } -> concepts::same_as_any<Value_t<NDF>, Spectrum_t<NDF>>;

      //! \brief mask has default value
      { ndf.G1(std::declval<Vec3d_t<NDF>>(), std::declval<Vec3d_t<NDF>>()) } -> concepts::same_as_any<Value_t<NDF>, Spectrum_t<NDF>>;
    };


    /********************************************************************/
    /*! \brief ndf archetype for concept checking

      \tparam CONF = specify config if a particular config is required.  Default is archetype::config
      *******************************************************************/
    namespace archetype {
      template<typename CONF=config> requires concepts::config<CONF>
        struct ndf
      {
        using Config = CONF;
        static constexpr string_literal name = "archetype::ndf";
        Value_t<Config> eval(Vec3d_t<Config>, Mask_t<Config> = true) const;
        Vec3d_t<Config> sample(Vec3d_t<Config>, Vec2d_t<Config>, Mask_t<Config> = true) const;
        Value_t<Config> pdf(Vec3d_t<Config>, Vec3d_t<Config>, Mask_t<Config> = true) const;
        Value_t<Config> G1(Vec3d_t<Config>, Vec3d_t<Config>, Mask_t<Config> = true) const;
      };
    } // end archetype namespace

    BBM_CHECK_CONCEPT(concepts::ndf, archetype::ndf<>);
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_NDF_CONCEPT_H_ */

