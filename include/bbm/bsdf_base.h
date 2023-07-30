#ifndef _BBM_BSDF_BASE_H_
#define _BBM_BSDF_BASE_H_

#include <string>

#include "concepts/bsdf.h"
#include "util/vector_util.h"
#include "bbm/bsdfmodel.h"


/***********************************************************************/
/*! \file bsdf_base.h
    \brief Abstract base definition of a BSDF (with virtual functions)

    The key difference between a bsdfmodel and the bsdf_model below
    is that bsdfmodel does not feature virtual functions, and thus which
    function to call is deciced at compile time.  bsdf_base on the other
    hand declares all methods as virtual, and thus which method to call
    is decided that run-time.

************************************************************************/

namespace bbm {

  /********************************************************************/
  /* \brief Abstract base class of BSDFs (with virtual methods)

     Each BSDF implements four __virtual__ method classes (each having two
     methods ending on _radiance or _importance):

    + \b eval: evaluates the BSDF given an in and out direction.
    + \b sample: samples the in direction with a pdf proportional BSDF given
    an out direction and 2 random values
    + \b pdf: the PDF of sampling a given in and out direction
    + \b albedo: the albedo of the BSDF given an out direction 

     In addition virtual utility methods are included:
    + \b toString: fancy print the bsdf
    + \b parameter_values : get the bsdf model parameters
    + \b parameter_default_values : get the default bsdf model parameters
    + \b parameter_lower_bound: lower bound of the bsdf model parameters
    + \b parameter_upper_bound: upper bound of the bsdf model parameters

    A bsdf_base is also a bsdfmodel as it immplements the full interface.

    Similar as with bsdfmodels, brdfs follow the in-out convention of
    Eric Veach, namely 'in' is the sampled direction, and transport flows
    from -in to out.  The physical meaning of 'in' and 'out' switches
    between the regular bsdf (Radiance) and the adjoint bsdf (Importance).

    Implements: concepts::bsdf
  **********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
  struct bsdf_base
  {
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = "bsdf_base";
    BBM_BSDF_FORWARD;
    
    //! \brief empty virtual destructor
    virtual ~bsdf_base(void) {}
    
    /*******************************************************************/
    /*! \brief Evaluate the BSDF given an in and out direction

      \param in  = incoming direction of transport
      \param out = exitant direction of transport
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the resulting Spectrum of the evaluation.

      IMPORTANT: The foreshortning (i.e., cosine) is __NOT__ included.
    ********************************************************************/
    virtual Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const = 0;
    
    /*******************************************************************/
    /*! \brief Samples the BSDF given an out direction and 2 random variables.

      \param out = the outgoing direction
      \param xi = two random variables stored in a Vec2d used to sample
      \param component = which reflectance component to sample
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns a BsdfSample that contains the sampled direction
               and the corresponding pdf.
    ********************************************************************/
    virtual BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const = 0;

    
    /*******************************************************************/
    /*! \brief The pdf of the in-out direction combination.

      \param in = the incoming sampled direction
      \param out = the exitant (given) direction
      \param component = which reflectance component was sampled
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the pdf of samling the in-out direction conbination.
    ********************************************************************/   
    virtual Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const = 0;

    
    /*******************************************************************/
    /*! \brief the (approximate) hemispherical reflectance of the BSDF

      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the approximate hemispherical reflectance (Spectrum) of the BSDF
    ********************************************************************/
    virtual Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const = 0;
    

    /*******************************************************************/
    /*! \brief Fancy print the BSDF to a string
     *******************************************************************/
    virtual std::string toString(void) const = 0;

    /*******************************************************************/
    /*! @{ \name Parameter Enumeration
     *******************************************************************/
    virtual bbm::vector<Value&> parameter_values(bsdf_attr flags=bsdf_attr::All) = 0;
    virtual bbm::vector<const Value&> parameter_values(bsdf_attr flags=bsdf_attr::All) const = 0;
    virtual bbm::vector<Value> parameter_default_values(bsdf_attr flags=bsdf_attr::All) const = 0;
    virtual bbm::vector<Value> parameter_lower_bound(bsdf_attr flags=bsdf_attr::All) const = 0;
    virtual bbm::vector<Value> parameter_upper_bound(bsdf_attr flags=bsdf_attr::All) const = 0;
    //! @}
    
  };

  /////////////////////
  // ostream support
  /////////////////////
  template<typename BSDF> requires bbm::concepts::bsdf<BSDF>
    std::ostream& operator<<(std::ostream& s, const BSDF& bsdf)
  {
    s << bsdf.toString();
    return s;
  }

} // end bbm namespace

#endif /* _BBM_BSDF_BASE_H_ */
