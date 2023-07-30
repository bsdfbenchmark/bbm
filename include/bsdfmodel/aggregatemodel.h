#ifndef _BBM_AGGREGATEMODEL_H_
#define _BBM_AGGREGATEMODEL_H_

#include <numeric>

#include "concepts/macro.h"
#include "concepts/bsdfmodel.h"

#include "util/constforeach.h"

/************************************************************************/
/*! \file aggregatemodel.h
    \brief The sum of different BSDF models.
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief The sum of different BSDF models.

    \tparam MODELS = list of BSDF models over which this model aggregates
  ***********************************************************************/
  template<typename... MODELS> requires (sizeof...(MODELS) >= 2) && concepts::matching_config<MODELS...> && (concepts::bsdfmodel<MODELS> && ...)
    struct aggregatemodel : public MODELS...
  {
    BBM_BASETYPES(MODELS...);
    BBM_IMPORT_CONFIG( std::tuple_element_t<0, std::tuple<MODELS...>> );
    static constexpr string_literal name = "AggregateModel";
    BBM_BSDF_FORWARD;
    
    //! \brief Default constructor
    aggregatemodel(void) : MODELS()... {}
    
    //! \brief Construction from each model
    aggregatemodel(const MODELS&... models) : MODELS(models)... {}
    
    //! \brief Copy constructor
    aggregatemodel(const aggregatemodel<MODELS...>& src) : MODELS( static_cast<MODELS>(src) )... {}
    
    //! \brief Assignment operator
    aggregatemodel<MODELS...>& operator=(const aggregatemodel<MODELS...>& src)
    {
      ((static_cast<MODELS&>(*this) = static_cast<MODELS>(src)), ...);
      return *this;
    }


    /********************************************************************/
    /*! \brief Evaluate the BSDF for a given in out direction

      \param in = incident direction
      \param out = outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = masking of lanes (e.g., for Packet eval)
      \returns Evaluation of the BSDF per spectrum.
      
      Evaluation is performed by evaluating each child model, and adding the
      results.
    *********************************************************************/
    Spectrum  eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      return (MODELS::eval(in, out, component, unit, mask) + ...);
    }

    /********************************************************************/
    /*! \brief Sample the aggregate BSDF given a direction and two random variables.

      \param out = outgoing direction
      \param xi = two random variables stored in a Vec2d
      \param component = which reflectance component to sample
      \param unit = unit of computation
      \param mask = masking of lanes.
      \returns A bsdfSample containing the sampled direction and the corresponding pdf.
      
      Selects one BSDF model to sample based with a probability propertional
      to the hemispherical reflectances of each BSDF.  The selected BSDF model
      is then sampled, and its sampled direction is returned.  The PDF is
      computed as the weighted sum of each BSDF model's PDF and the
      reflectance weights.
    *********************************************************************/
    BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      BsdfSample result = {0,0,bsdf_flag::None};
        
      // Gather weight per PDF (weight ~ reflectance)
      std::array<Value, sizeof...(MODELS)> weights = {bbm::hsum(MODELS::reflectance(out, component, unit, mask)) ...};
      
      // Normalization factor
      auto sum = std::accumulate(std::begin(weights), std::end(weights), Value(0));
      
      // Iterate through all MODELS and select the model to sample based on xi[0]:
      // 1) there are few models, so there is not much to gain from a binary search
      // 2) when using packet types, we likely need to evaluate all models.
      auto weight = std::begin(weights);
      auto xi0 = xi[0] * sum;

      CONSTFOREACH(MODEL, MODELS,
      {
        // check if xi0 falls in [0...weight]
        Mask m = mask && (xi0 >= 0 && xi0 <= *weight);

        // Normalize xi0 to [0...1]
        Value normalized_xi0 = bbm::select( m && (*weight > Constants::Epsilon()), xi0 / *weight, 0);

        // If in range, sample; otherwise keep current solution
        // NOTE: Using static_cast<MODEL*>(this)-> instead of MODEL::otherwise
        //       'this' might not be captured in CONSTFOREACH lambda.
        result = bbm::select(m, static_cast<const MODEL*>(this)->sample(out, Vec2d(normalized_xi0, xi[1]), component, unit, m), result);

        // prepare for next MODEL
        xi0 -= *weight;
        weight++;
      });
      
      // Compute PDF
      std::array<Value, sizeof...(MODELS)> pdfs = {MODELS::pdf(result.direction, out, component, unit, mask) ...};
      result.pdf = bbm::select(sum > Constants::Epsilon(), std::inner_product(std::begin(pdfs), std::end(pdfs), std::begin(weights), Value(0)) / sum, 0);
      
      // Done.
      return result;
    }
    

    /********************************************************************/
    /*! \brief Compute the pdf given an in and out direction

      \param in = the incoming (sampled) direction
      \param out = the outgoing (given) direction
      \param component = which reflectance component was sampled
      \param unit = unit of computation
      \param mask = masking of lanes
      \returns the PDF that the incoming direction would be sampled given the outgoing direction.
      
      The PDF is computed as the reflectance weighted sum of all PDFs.
    *********************************************************************/ 
    Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      // Check masks
      if(bbm::none(mask)) return 0;
      
      // Gather all PDFs
      std::array<Value, sizeof...(MODELS)> pdfs = {MODELS::pdf(in, out, component, unit, mask) ...};
      
      // Gather weight per PDF (weight ~ reflectance)
      std::array<Value, sizeof...(MODELS)> weights = {bbm::hsum(MODELS::reflectance(out, component, unit, mask)) ...};
      
      // Return normalized weighted sum
      auto sum = std::accumulate(std::begin(weights), std::end(weights), Value(0));
      return bbm::select(sum > Constants::Epsilon(), std::inner_product(std::begin(pdfs), std::end(pdfs), std::begin(weights), Value(0)) / sum, 0);
    }


    /********************************************************************/
    /*! \brief Return the (approximate) hemispherical reflectance of the BSDF
      
      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = masking of lanes
      \returns the approximate reflectance of the BSDF for a given direction
      
      The hemispherical reflectance is computed as the sum of the underlying
      BSDF models.
    *********************************************************************/
    Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      // Check mask
      if(bbm::none(mask)) return Spectrum(0);

      // Sum reflectances
      return (MODELS::reflectance(out, component, unit, mask) + ...);
    }

    /********************************************************************/
    /*! \brief toString to overwrite default printing behavior
     ********************************************************************/
    inline std::string toString(void) const
    {
      std::string str("(");
      CONSTFOREACH(MODEL, MODELS,
      {
        if(!str.empty()) str += ", ";
        str += bbm::toString( static_cast<const MODEL&>(*this) );
      });
      
      return  str + ")";
    }
  };


  /**********************************************************************/
  /*! \brief Method for simplifying the creation of an aggregate model

    \param models = comma separated list of models.

    Based on the list of BSDF models, an aggregate model is created and
    initialized with the parameters of the models in the parameter list.
  ***********************************************************************/
  template<typename... MODELS> requires (concepts::bsdfmodel<MODELS> && ...)
    aggregatemodel<MODELS...> aggregate(const MODELS&... models) { return aggregatemodel<MODELS...>(models...); }
  
} // end bbm namespace

#endif /* _BBM_AGGREGATEMODEL_H_ */
