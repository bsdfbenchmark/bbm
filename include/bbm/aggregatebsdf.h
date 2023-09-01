#ifndef _BBM_AGGREGATEBSDF_H_
#define _BBM_AGGREGATEBSDF_H_

#include <vector>
#include <numeric>
#include <utility>
#include <algorithm>

#include "concepts/config.h"
#include "concepts/stringconvert.h"

#include "util/multirange_for.h"

#include "bbm/bsdf.h"
#include "bbm/bsdf_ptr.h"
#include "bsdfmodel/aggregatemodel.h"



/************************************************************************/
/*! \file aggregatebsdf.h
    \brief A run-time aggregation of BSDFs.

    In contrast to aggregatemodel which aggregates BSDF models at compile
    time, aggregatebsdf aggregates BSDFs at run time by using virtual
    functions.
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief A BSDF that is the aggregate of different BSDFs.

    Note: the aggregatebsdf does not own the BSDFs, instead it takes
    bsdf_ptrs as input.  
  ***********************************************************************/
  template<typename CONF, string_literal NAME="Aggregate"> requires concepts::config<CONF>
    class aggregatebsdf : public bsdf_base<CONF>
  {
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = NAME;
    BBM_BSDF_FORWARD;
    
    //! \brief Construct an aggregate bsdf from a list of bsdf_ptrs.
    template<typename... BSDFS> requires concepts::matching_config<Config, BSDFS...> && (concepts::bsdf_ptr<BSDFS> && ...)
      aggregatebsdf(const BSDFS&... src) : _bsdfs({src...}) {}
    
    //! \brief Construct an aggregate bsdf from iterators to list of 'bsdf_ptrs'
    template<typename ITR> requires std::input_or_output_iterator<ITR> && concepts::bsdf< std::iter_reference_t<ITR> > && concepts::matching_config< std::iter_reference_t<ITR> > && concepts::bsdf_ptr< std::iter_value_t<ITR> >
      aggregatebsdf(const ITR& begin, const ITR& end)
    {
      // copy
      for(ITR itr = begin; itr != end; ++itr)
        _bsdfs.push_back(*itr);
    }
    
    //! \brief Copy Constructor
    aggregatebsdf(const aggregatebsdf<Config>& src) : _bsdfs(src._bsdfs) {}

    //! \brief Assignment Operator
    aggregatebsdf<Config>& operator=(const aggregatebsdf<Config>& src) { _bsdfs = src._bsdfs; return *this; }

    /********************************************************************/
    /*! \brief  Sum the BSDF evaluations for a given in out direction

      \param in = incident direction
      \param out = outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = masking of lanes (e.g., for Packet eval)
      \returns Evaluation of the BSDF per spectrum.
      
      Evaluation is performed by evaluating each BSDF in _bsdfs, and adding
      the results.
    *********************************************************************/
    virtual Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      // accumulate eval of each bsdf.
      return std::accumulate(_bsdfs.begin(), _bsdfs.end(), Spectrum(0), [&](const Spectrum& result, auto& bsdf)
      {
        return result + bsdf->eval(in, out, component, unit, mask);
      });      
    }
    

    /********************************************************************/
    /*! \brief Sample the aggregate BSDF given a direction and two random variables.

      \param out = outgoing direction
      \param xi = two random variables stored in a Vec2d
      \param component = which reflectance component to sample
      \param unit = unit of computation
      \param mask = masking of lanes.
      \returns A bsdfSample containing the sampled in direction and the corresponding pdf.
      
      Selects one BSDF in _bsdf to sample based with a probability
      propertional to the hemispherical reflectances of each BSDF.  The selected BSDF is then
      sampled, and its sampled direction is returned.  The PDF is computed as
      the weighted sum of each BSDF's PDF and the reflectance weights.
    *********************************************************************/
    virtual BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      BsdfSample result;
      
      // get weights & sum
      std::vector<Value> weights;
      Value sum = 0;
      for(auto& bsdf : _bsdfs)
      {
        weights.push_back( bbm::hsum(bsdf->reflectance(out, component, unit, mask)) );
        sum += weights.back();
      }

      // quick bail out
      mask &= (sum > Constants::Epsilon());
      if(bbm::none(mask)) return result;

      // Iterate through all _bsdfs and select a bsdf based on xi[0]
      // 1) there are likely few bsdfs in _bsdfs, so a binary search is overkill
      // 2) when using packet types, we likely need to sample all models anyway
      auto residual = xi[0] * sum;

      multirange_for([&](auto& bsdf, auto& weight)
      {
        // within range?
        Mask temp_mask = mask && ((residual >= 0) && (residual <= weight));

        // Normalize residual
        Value normalized_residual = bbm::select(temp_mask && (weight > Constants::Epsilon()), residual / weight, 0);

        // sample if temp_mask is true
        result = bbm::select(temp_mask, bsdf->sample(out, Vec2d( normalized_residual, xi[1] ), component, unit, temp_mask), result);

        // next sample
        residual -= weight;
      }, _bsdfs, weights);
      
      // update pdf with weighted sum
      result.pdf = 0;
      multirange_for([&](auto& bsdf, auto& weight)
      {
        result.pdf += weight * bsdf->pdf(result.direction, out, component, unit, mask) / sum;
      }, _bsdfs, weights);
      
      // Done.
      return result;
    }

    
    /********************************************************************/
    /*! \brief Compute the pdf given an in and out direction

      \param in = the incoming sampled direction
      \param out = the outgoing (given) direction
      \param component = which reflectance component was sampled
      \param unit = unit of computation
      \param mask = masking of lanes
      \returns the PDF that the incoming direction would be sampled given the outgoing direction.
      
      The PDF is computed as the hemispherical reflectance  weighted sum of all PDFs.
    *******************************************************************/
    virtual Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      
      // get weights & sum
      std::vector<Value> weights;
      Value sum = 0;
      for(auto& bsdf : _bsdfs)
      {
        weights.push_back( bbm::hsum(bsdf->reflectance(out, component, unit, mask)) );
        sum += weights.back();
      }

      // quick bail out
      mask &= (sum > Constants::Epsilon());
      if(bbm::none(mask)) return Value(0);

      // compute and return pdf as weighted sum
      Value pdf = 0;
      multirange_for([&](auto& bsdf, auto& weight)
      {
        pdf += weight * bsdf->pdf(in, out, component, unit, mask) / sum;
      }, _bsdfs, weights);

      // Done.
      return pdf;
    }
    
    
    /*****************************************************************/
    /*! \brief Return the (approximate) hemispherical reflectance of the BSDF
      
      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = masking of lanes
      \returns the approximate hemispherical reflectance of the BSDF for a given outgoing direction
      
      The reflectance is computed as the sum of the underlying BSDF's hemispherical reflectance.
    ******************************************************************/
    virtual Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      // accumulate reflectance over all bsdfs.
      Spectrum sum(0);
      for(auto& bsdf : _bsdfs)
        sum += bsdf->reflectance(out, component, unit, mask);

      // Done.
      return sum;
    }


    /******************************************************************/
    /*! \brief Convert the aggregatebsdf to string
     ******************************************************************/
    virtual std::string toString(void) const override final
    {
      return bbm::toString(name) + bbm::toString(_bsdfs);
    }

    /*******************************************************************/
    /*! \brief Construct the aggregate bsdf from a string
     *******************************************************************/
    static inline aggregatebsdf fromString(const std::string& str)
    {
      auto [key, value] = string::get_keyword(str);

      // check name
      if(key != std::string(aggregatebsdf::name)) throw std::invalid_argument(std::string("BBM: mismatched object name ") + key + ", expected " + std::string(aggregatebsdf::name));
      
      // read vector of bsdf_ptrs
      auto bsdfs = bbm::fromString<bbm::vector<bsdf_ptr<Config>>>(value);

      // Done: create from vector.
      return aggregatebsdf(std::begin(bsdfs), std::end(bsdfs));
    }
    
    /*******************************************************************/
    /*! \name Parameter Enumeration
     *******************************************************************/
    virtual bbm::vector<Value&> parameter_values(bsdf_attr flags=bsdf_attr::All) override final
    {
      bbm::vector<Value&> result;
      for(auto& bsdf : _bsdfs)
      {
        auto param = bbm::parameter_values(bsdf, flags);
        result.insert(result.end(), param.begin(), param.end());
      }
      return result;
    }
      
    virtual bbm::vector<const Value&> parameter_values(bsdf_attr flags=bsdf_attr::All) const override final
    {
      bbm::vector<const Value&> result;
      for(auto& bsdf : _bsdfs)
      {
        auto param = bbm::parameter_values(std::as_const(bsdf), flags);
        result.insert(result.end(), param.begin(), param.end());
      }
      return result;
    }
    
    virtual bbm::vector<Value> parameter_default_values(bsdf_attr flags=bsdf_attr::All) const override final
    {
      bbm::vector<Value> result;
      for(auto& bsdf : _bsdfs)
      {
        auto param = bbm::parameter_default_values(bsdf, flags);
        result.insert(result.end(), param.begin(), param.end());
      }
      return result;
    }
    
    virtual bbm::vector<Value> parameter_lower_bound(bsdf_attr flags=bsdf_attr::All) const override final
    {
      bbm::vector<Value> result;
      for(auto& bsdf : _bsdfs)
      {
        auto param = bbm::parameter_lower_bound(bsdf, flags);
        result.insert(result.end(), param.begin(), param.end());
      }
      return result;      
    }
    
    virtual bbm::vector<Value> parameter_upper_bound(bsdf_attr flags=bsdf_attr::All) const override final
    {
      bbm::vector<Value> result;
      for(auto& bsdf : _bsdfs)
      {
        auto param = bbm::parameter_upper_bound(bsdf, flags);
        result.insert(result.end(), param.begin(), param.end());
      }
      return result;      
    }
    //! @}
    
  private:
    //////////////////
    // Data Members //
    //////////////////
    std::vector<bsdf_ptr<Config>> _bsdfs;
  };

  
  /*********************************************************************/
  /*! @{ \brief Helper methods for simplifying the creation of a aggregate bsdf

    This is a companion method to aggregate in aggregatemodel.h. Based on
    the available information at compile time, aggrgate chooses the most
    optimal aggregate bsdf defintion:

    + If all arguments are BSDF models, then the result is an aggregatemodel
    + If all arguments are bsdf<BSDFMODEL>, then the result is an bsdf<aggregatemodel>
    + If all arguments are bsdf_ptr, then the result is an aggregatebsdf
    + If all arguments are pased via an iterator, then the result is an aggregatebsdf.

    In the latter two cases, the underlying BSDFMODEL information is only
    known at run-time, and hence, the run-time aggregatebsdf is required. In
    the first case, because the user specifies models directly, the result is
    expected to be a model too.  Finally, a bsdf<aggregatemodel> is less overhead
    than an aggregatebsdf, and hence, it is preferred when at compile time
    the BSDF models are known.

    Note: ownership of the data depenends on the underlying model. In the
    case of aggregate bsdfs, ownership still lies with the original BSDF.
  ***********************************************************************/
  template<typename... BSDFMODELs> requires (concepts::bsdfmodel<BSDFMODELs> && ...)
  auto aggregate(const bsdf<BSDFMODELs>&... src)
  {
    return bsdf<aggregatemodel<BSDFMODELs...>>( (dynamic_cast<const BSDFMODELs&>(src))... );
  }

  template<typename... CONFs> requires (concepts::config<CONFs> && ...)
    auto aggregate(const bsdf_ptr<CONFs>&... src)
  {
    using cfg = get_config< std::tuple_element_t<0, std::tuple<CONFs...>> >;
    return aggregatebsdf<cfg>(src...);
  }


  template<typename ITR> requires (std::input_or_output_iterator<ITR> && concepts::bsdf_ptr<std::iter_reference_t<ITR>>)
    auto aggregate(const ITR& begin, const ITR& end)
  {
    using cfg  = get_config< std::iter_reference_t<ITR> >;
    return aggregatebsdf<cfg>(begin, end);
  }
  //! @}
  
} // end bbm namespace

#endif /* _BBM_AGGREGATEBSDF_H_ */
