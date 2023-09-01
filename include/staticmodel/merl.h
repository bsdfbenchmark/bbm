#ifndef _BBM_MERL_H_
#define _BBM_MERL_H_

#include <fstream>
#include <utility>

#include "concepts/macro.h"
#include "concepts/bsdfmodel.h"

#include "bbm/config.h"
#include "bbm/ndf_sampler.h"

#include "core/error.h"
#include "core/color.h"
#include "core/spherical.h"

#include "linearizer/merl_linearizer.h"
#include "bsdfmodel/lambertian.h" 


/************************************************************************/
/*! \file merl.h
    \brief Support for the MERL-MIT brdf database: https://doi.org/10.1145/882262.882343

    Loading and evaluating of MERL-MIT measured BRDF as a bsdf model
*************************************************************************/

namespace bbm
{

  /*********************************************************************/
  /*! \brief MERL-MIT sampled reflectance

    Stored and provides access to the MERL-MIT measured BRDFs.

    Sample, pdf, and reflectance are implemented by placeholders.
  **********************************************************************/
  template<typename CONF, string_literal NAME="MerlData"> requires concepts::config<CONF>
    class merl_data
  {
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = NAME;
    //BBM_BSDF_FORWARD;

    //! Empty Constructor
    merl_data(void) = delete;

    //! Copy constructor
    merl_data(const merl_data& src) = default;

    //! Assignment operator
    merl_data& operator=(const merl_data& src) = default;

    /********************************************************************/
    /*! \brief Import Constructor
      
      \param filename = name of MERL BRDF file
       ******************************************************************/
    BBM_CONSTRUCTOR( merl_data, args, arg<const std::string&, "filename"> ) : _filename(args.value("filename"_arg))
    {
      import();
    }


    /********************************************************************/
    /*! \brief Evaluate the BSDF given an in and out direction

      \param in  = incoming direction of transport
      \param out = exitant direction of transport
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the resulting Spectrum of the evaluation.

      IMPORTANT: The foreshortning (i.e., cosine) is __NOT__ included.
    ********************************************************************/
    Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      Spectrum result(0);
      
      // matching flag
      mask &= is_set(component, bsdf_flag::All);

      // above surface?
      mask &= (vec::z(in) >= 0) && (vec::z(out) >= 0);

      // Quick exit
      if(bbm::none(mask)) return result;

      // Get index in data
      Size_t index = _linearizer(in, out, mask);

      // Get data (TODO: conversion from color to Spectrum)
      return bbm::lookup<Spectrum>(_data, index, bbm::cast<index_mask_t<Value>>(mask));
    }
    
    /*******************************************************************/
    /*! \brief Sample the sanpled BSDF given a direction and two random variables.

      \param out = outgoing direction
      \param xi = two random variables stored in a Vec2d
      \param component = which reflectance component to sample
      \param unit  =unit of computation
      \param mask = mask to enable/disable lanes
      \returns A bsdfSample containing the sampled direction and the corresponding pdf.
    *********************************************************************/
    BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      // matching flag
      mask &= is_set(component, bsdf_flag::All);

      // Placeholder
      return lambertian<Config>().sample(out, xi, bsdf_flag::Diffuse, unit, mask);
    }
      
    /********************************************************************/
    /*! \brief Compute the pdf given an in and out direction

      \param in = the incoming direction
      \param out = the outgoing direction
      \param component = which reflectance component was sampled
      \param unit = unit of computation
      \param mask = enable/disable lanes.
      \returns the PDF that the outgoing direction would be sampled given the incoming direction.
    *********************************************************************/
    Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      // matching flag
      mask &= is_set(component, bsdf_flag::All);

      // Placeholder
      auto sample = lambertian<Config>().pdf(in, out, bsdf_flag::Diffuse, unit, mask);
      sample.flag = bbm::select(sample.flag == bsdf_flag::Diffuse, BsdfFlag(bsdf_flag::All), BsdfFlag(bsdf_flag::None));

      // Done.
      return sample;
    }

    /*****************************************************************/
    /*! \brief Return the (approximate) hemispherical reflectance of the BSDF

      \param out = the outgoing direction (ignored)
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = enable/disable lanes
      \returns the approximate hemispherical reflectance of the BSDF for a given incident direction
    ******************************************************************/
    Spectrum reflectance(const Vec3d& /*out*/, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // matching flag
      mask &= is_set(component, bsdf_flag::All);

      // Placeholder
      return bbm::select(mask, Spectrum(1), Spectrum(0));
    }

    /********************************************************************/
    /*! \brief Custom string conversion
     ********************************************************************/
    inline std::string toString(void) const
    {
      return std::string(name) + "(" + bbm::toString(_filename) + ")";
    }

  private:
    /********************************************************************/
    /*! \brief Import MERL data from file

      Load the MERL-MIT BRDF data from path/filename.  Will throw a run-time
      exception if not found, or if the format is not a MERL-MIT format.
    *********************************************************************/
    inline void import(void)
    {
      // Check file
      std::ifstream ifs(_filename, std::ios_base::binary);
      if(!ifs) throw std::runtime_error("BBM: unable to open MERL BRDF: " + bbm::toString(_filename));

      uint32_t thetaH, thetaD, phiD;
      ifs.read(reinterpret_cast<char*>(&thetaH), sizeof(uint32_t));
      ifs.read(reinterpret_cast<char*>(&thetaD), sizeof(uint32_t));
      ifs.read(reinterpret_cast<char*>(&phiD), sizeof(uint32_t));

      if(thetaH != 90 || thetaD != 90 || phiD != 180) throw std::runtime_error("BBM: not a recognized MERL BRDF: " + bbm::toString(_filename));

      // create linearizer
      _linearizer = merl_linearizer<Config>( vec2d<Size_t>(1, 90), vec2d<Size_t>(180, 90) );

      // read data
      size_t size = thetaH * thetaD * phiD; 
      auto buffer = std::make_unique<double[]>(3 * size);
      auto buffer_ref = buffer.get();
      ifs.read(reinterpret_cast<char *>(buffer_ref), 3 * size * sizeof(double));
      
      // redistribute (remove negative values and apply white balance)
      _data.clear();
      _data.reserve(size);
      
      for(size_t idx=0; idx != size; ++idx)
        _data.push_back( color<double>( bbm::max(0, buffer_ref[idx] * 1.0 / 1500.0),
                                        bbm::max(0, buffer_ref[size + idx] * 1.15 / 1500.0),
                                        bbm::max(0, buffer_ref[2*size + idx] * 1.66 / 1500.0)
                                       ) );

      // Done.
    }

    ///////////////////////
    //! @{ Class members
    ///////////////////////
    std::string _filename; 
    merl_linearizer<Config> _linearizer;
    std::vector<color<double>> _data;
    //! @}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, merl_data<config>);


  /**********************************************************************/
  /*! \brief Data-driven MERL BSDF model with data-driven backscatter based
      importance sampling
   **********************************************************************/
  template<typename Config> requires concepts::config<Config>
    using merl = ndf_sampler<merl_data<Config, "Merl">, 90, 1>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, merl<config>);
  
} // end bbm namespace

#endif /* _BBM_MERL_H_ */

BBM_EXPORT_BSDFMODEL(bbm::merl)

