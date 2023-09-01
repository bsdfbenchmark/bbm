#ifndef _BBM_BSDF_H_
#define _BBM_BSDF_H_

#include "concepts/bsdf.h"

#include "bbm/config.h"
#include "bbm/bsdf_base.h"
#include "bbm/bsdf_enumerate.h"

/************************************************************************/
/*! \file bsdf.h
    \brief Connects a BSDF model and a BSDF.

    This class provides a simple interface for connecting BSDF models
    (without virtual functions) to a BSDF (with virtual functions).  In
    essence, this class just passes through the method call to the BSDF 
    model.
*************************************************************************/

namespace bbm {

  
  /**********************************************************************/
  /*! \brief BSDF implementation of a BSDF model

    \tparam BSDFMODEL = the BSDF model to transform into a BSDF
  ***********************************************************************/
  template<typename BSDFMODEL> requires concepts::bsdfmodel<BSDFMODEL>
    class bsdf : virtual public bsdf_base< get_config<BSDFMODEL>>, public BSDFMODEL
  {
  public:
    BBM_BASETYPES(BSDFMODEL);
    BBM_IMPORT_CONFIG( BSDFMODEL );
    static constexpr string_literal name = "Bsdf";
    BBM_BSDF_FORWARD;
    
    //! \brief Inherit all constructors
    using BSDFMODEL::BSDFMODEL;

    //! \brief Construct directly from a BSDF model
    bsdf(const BSDFMODEL& model) : BSDFMODEL(model) {}

    //! \brief Assignment operator
    using BSDFMODEL::operator=;
    
    /********************************************************************/
    /*! \brief Virtual passthrough of eval function
        
      \param in  = incoming direction
      \param out = exitant direction
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the resulting Spectrum of the evaluation.
    ********************************************************************/
    virtual Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      return BSDFMODEL::eval(in, out, component, unit, mask);
    }

      
    /********************************************************************/
    /*! \brief Virtual passthrough of sample function

      \param out = the outgoing direction
      \param xi = two random variables stored in a Vec2d used to sample
      \param component = which reflectance component to sample
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns a BsdfSample that contains the sampled direction
      and the corresponding pdf.
    ********************************************************************/
    virtual BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      return BSDFMODEL::sample(out, xi, component, unit, mask);
    }
    

    /********************************************************************/
    /*! \brief Virtual passthrough of the pdf function

      \param in = the incoming sampled direction
      \param out = the exitant (given) direction
      \param component = which reflectance component was sampled
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the pdf of samling the in-out direction conbination.
    *********************************************************************/   
    virtual Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      return BSDFMODEL::pdf(in, out, component, unit, mask);
    }

      
    /********************************************************************/
    /*! \brief Virtual passthrough of the hemispherical reflectance function

      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the approximate hemispherical reflectance (Spectrum) of the BSDF
    *********************************************************************/     
    virtual Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      return BSDFMODEL::reflectance(out, component, unit, mask);
    }
    
      
    /********************************************************************/
    /*! \brief Pipe toString to BSDFMODEL
     ********************************************************************/
    virtual std::string toString(void) const override final
    {
      return bbm::toString( static_cast<const BSDFMODEL&>(*this) );
    }

    /*********************************************************************/
    /*! \brief construct the bsdf from a string
     *********************************************************************/
    static bsdf fromString(const std::string& str)
    {
      return bsdf( bbm::fromString<BSDFMODEL>(str) );
    }
    
    /*********************************************************************/
    /*! \name Parameter Enumeration
     *********************************************************************/
    virtual bbm::vector<Value&> parameter_values(bsdf_attr flags=bsdf_attr::All) override final
    {
      return bbm::parameter_values(dynamic_cast<BSDFMODEL&>(*this), flags);
    }
    
    virtual bbm::vector<const Value&> parameter_values(bsdf_attr flags=bsdf_attr::All) const override final
    {
      return bbm::parameter_values(dynamic_cast<const BSDFMODEL&>(*this), flags);
    }
    
    virtual bbm::vector<Value> parameter_default_values(bsdf_attr flags=bsdf_attr::All) const override final
    {
      return bbm::parameter_default_values(dynamic_cast<const BSDFMODEL&>(*this), flags);
    }
    
    virtual bbm::vector<Value> parameter_lower_bound(bsdf_attr flags=bsdf_attr::All) const override final
    {
      return  bbm::parameter_lower_bound(dynamic_cast<const BSDFMODEL&>(*this), flags);
    }
    
    virtual bbm::vector<Value> parameter_upper_bound(bsdf_attr flags=bsdf_attr::All) const override final
    {
      return  bbm::parameter_upper_bound(dynamic_cast<const BSDFMODEL&>(*this), flags);
    }
    //! @}
  };

  BBM_CHECK_CONCEPT(concepts::bsdf, bsdf<bsdfmodel<>>);
  
} // end bbm namespace

#endif /* _BBM_BSDF_H_ */
