#ifndef _BBM_BSDFMODEL_CONCEPT_H_
#define _BBM_BSDFMODEL_CONCEPT_H_

#include "concepts/config.h"
#include "concepts/named.h"

#include "util/string_literal.h"

#include "bbm/unit.h"
#include "bbm/config.h"

/************************************************************************/
/*! \file bsdfmodel.h

  \brief bsdfmodel contract
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief bsdfmodel concept

      Each bsdfmodel requires:
      + concepts::config
      + concepts::named
      + Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
      + BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
      + Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
      + Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    *********************************************************************/
    template<typename BSDFMODEL>
      concept bsdfmodel = requires(const BSDFMODEL& model)
    {
      requires concepts::config<BSDFMODEL>;
      requires concepts::named<BSDFMODEL>;
      
      /******************************************************************/
      /*! \brief Evaluate the BSDF given an in and out direction

        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        
        \param in  = incident direction of transport
        \param out = outgoing direction of transport
        \param component = which reflectance component to eval [default = bsdf_flag::All]
        \param unit = unit of computation [default = unit_t::Radiance]
        \param mask = mask to enable/disable lanes [default = true]
        \returns the resulting Spectrum of the evaluation.

        IMPORTANT: The foreshortning (i.e., cosine) is __NOT__ included.
      *******************************************************************/
      { model.eval(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>(), std::declval<unit_t>(), std::declval<Mask_t<BSDFMODEL>>()) } -> std::same_as<Spectrum_t<BSDFMODEL>>;

      //! \brief mask has default value.
      { model.eval(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>(), std::declval<unit_t>())} -> std::same_as<Spectrum_t<BSDFMODEL>>;

      //! \brief unit template parameter has default.
      { model.eval(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>())} -> std::same_as<Spectrum_t<BSDFMODEL>>;
      
      //! \brief component has default value.
      { model.eval(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec3d_t<BSDFMODEL>>()) } -> std::same_as<Spectrum_t<BSDFMODEL>>;
      

      /******************************************************************/
      /*! \brief Samples an incoming direction with a pdf proportional to the
        BSDF given an out direction and 2 random variables.

        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        
        \param out = the outgoing direction 
        \param xi = two random variables stored in a Vec2d used to sample
        \param component = which reflectance component to sample [default=bsdf_flag::All]
        \param unit = unit of computation [default=unit_t::Radiance]
        \param mask = mask to enable/disable lanes [default=true]
        \returns a BsdfSample that contains the sampled direction
                 and the corresponding pdf.
      *******************************************************************/
      { model.sample(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec2d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>(), std::declval<unit_t>(), std::declval<Mask_t<BSDFMODEL>>()) } -> std::same_as<BsdfSample_t<BSDFMODEL>>;

      //! \brief mask has default value
      { model.sample(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec2d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>(), std::declval<unit_t>()) } -> std::same_as<BsdfSample_t<BSDFMODEL>>;

      //! \brief unit template parameter has default
      { model.sample(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec2d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>()) } -> std::same_as<BsdfSample_t<BSDFMODEL>>;
      
      //! \brief component has default value
      { model.sample(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec2d_t<BSDFMODEL>>()) } -> std::same_as<BsdfSample_t<BSDFMODEL>>;
      

      /******************************************************************/
      /*! \brief The pdf of the in-out direction combination.

        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        
        \param in = the incoming direction (i.e., sampled direction)
        \param out = the exitant direction (i.e., the given direction)
        \param component = which reflectance component was sampled [default=bsdf_flag::All]
        \param unit = unit of computation [default=unit_t::Radiance]
        \param mask = mask to enable/disable lanes [default=true]
        \returns the pdf (Value) of sampling the in-out direction conbination.
      *******************************************************************/
      { model.pdf(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>(), std::declval<unit_t>(), std::declval<Mask_t<BSDFMODEL>>()) } -> std::same_as<Value_t<BSDFMODEL>>;

      //! \brief mask has default value
      { model.pdf(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>(), std::declval<unit_t>()) } -> std::same_as<Value_t<BSDFMODEL>>;

      //! \brief unit has default template value
      { model.pdf(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>()) } -> std::same_as<Value_t<BSDFMODEL>>;
      
      //! \brief component has default value
      { model.pdf(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<Vec3d_t<BSDFMODEL>>()) } -> std::same_as<Value_t<BSDFMODEL>>;
      

      /******************************************************************/
      /*! \brief the (approximate) hemispherical reflectance of the BSDF. This
        roughly corresponds to the expected value ratio between bsdf and pdf.

        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        
        \tparam UNIT = Radiance/Importance [default=unit_t::Radiance]
        \param out = the incident direction
        \param component = which reflectance component to eval [default=bsdf_flag::All]
        \param unit = unit of computation [default=unit_t::Radiance]
        \param mask = mask to enable/disable lanes [default=true]
        \returns the approximate hemispherical reflectance (Spectrum) of the BSDF
      *******************************************************************/
      { model.reflectance(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>(), std::declval<unit_t>(), std::declval<Mask_t<BSDFMODEL>>()) } -> std::same_as<Spectrum_t<BSDFMODEL>>;

      //! \brief UNIT has default template parameter
      { model.reflectance(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>(), std::declval<unit_t>()) } -> std::same_as<Spectrum_t<BSDFMODEL>>;
      
      //! \brief mask has default value
      { model.reflectance(std::declval<Vec3d_t<BSDFMODEL>>(), std::declval<BsdfFlag_t<BSDFMODEL>>()) } -> std::same_as<Spectrum_t<BSDFMODEL>>;
      
      //! \brief component has default value
      { model.reflectance(std::declval<Vec3d_t<BSDFMODEL>>()) } -> std::same_as<Spectrum_t<BSDFMODEL>>;
      
    };


    /********************************************************************/
    /*! \brief bsdfmodel archetype for concept checking

      \tparam CONF = config to check for. Default = archetype::config
    ********************************************************************/
    namespace archetype {
      template<typename CONF=config> requires concepts::config<CONF>
        struct bsdfmodel
      {
        using Config = CONF;
        static constexpr string_literal name = "archetype::bsdfmodel";
        Spectrum_t<Config> eval(Vec3d_t<Config>, Vec3d_t<Config>, BsdfFlag_t<Config> = bsdf_flag::All, unit_t = unit_t::Radiance, Mask_t<Config> = true) const;
        BsdfSample_t<Config> sample(Vec3d_t<Config>, Vec2d_t<Config>, BsdfFlag_t<Config> = bsdf_flag::All, unit_t = unit_t::Radiance, Mask_t<Config> = true) const;
        Value_t<Config> pdf(Vec3d_t<Config>, Vec3d_t<Config>, BsdfFlag_t<Config> = bsdf_flag::All, unit_t = unit_t::Radiance, Mask_t<Config> = true) const;
        Spectrum_t<Config> reflectance(Vec3d_t<Config>, BsdfFlag_t<Config> = bsdf_flag::All, unit_t = unit_t::Radiance, Mask_t<Config> = true) const;        
      };
    } // end archetype namespace

    BBM_CHECK_CONCEPT(concepts::bsdfmodel, archetype::bsdfmodel<>);
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_BSDFMODEL_CONCEPT_H_ */

