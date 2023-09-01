#ifndef _BBM_BSDF_PTR_H_
#define _BBM_BSDF_PTR_H_

#include "concepts/bsdf_ptr.h"
#include "concepts/stringconvert.h"

#include "util/pointer.h"
#include "core/error.h"
#include "bbm/bsdf.h"

/***********************************************************************/
/*! \file bsdf_ptr.h
    \brief A shared_ptr wrapper for bsdfs.

    Implements concepts::bsdf_ptr
************************************************************************/

namespace bbm
{
  template<typename CONF> requires concepts::config<CONF>
    struct bsdf_ptr : public virtual bsdf_base<CONF>
  {
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = "BsdfPtr";
    BBM_BSDF_FORWARD;
    
    //! \brief Default (empty) constructor; shared_ptr is set to nullptr.
    bsdf_ptr(void) = default;
  
    //! \brief Construct from pointer
    bsdf_ptr(const pointer<bsdf_base<Config>>& ptr) : _bsdf(ptr) {}

    //! \brief Copy constructor
    bsdf_ptr(const bsdf_ptr<CONF>& b) : _bsdf(b._bsdf) {}

    //! \brief Assignment operator
    bsdf_ptr& operator=(const bsdf_ptr& b)
    {
      _bsdf = b._bsdf;
      return *this;
    }
    
    //! \brief Get the internal shared pointer.
    inline const pointer<bsdf_base<Config>>& ptr(void) const { return _bsdf; }
    
    //! @{ \name Override the -> operator
    inline const pointer<bsdf_base<Config>>& operator->(void)
    {
      if(!_bsdf) throw bbm_incomplete_init;
      return _bsdf;
    }

    inline pointer<const bsdf_base<Config>> operator->(void) const
    {
      if(!_bsdf) throw bbm_incomplete_init;
      return _bsdf;
    }
    //! @}
    
    /******************************************************************/
    /*! \brief Pointer-dereference passthrough of eval function
        
      \param in  = incoming direction
      \param out = exitant direction
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the resulting Spectrum of the evaluation.
    *******************************************************************/
    virtual Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      return (*this)->eval(in, out, component, unit, mask);
    }

    
    /******************************************************************/
    /*! \brief Pointer-derefence passthrough of sample function

      \param out = the outgoing direction
      \param xi = two random variables stored in a Vec2d used to sample
      \param component = which reflectance component to sample
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns a BsdfSample that contains the sampled direction
      and the corresponding pdf.
    ******************************************************************/
    virtual BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      return (*this)->sample(out, xi, component, unit, mask);
    }

    
    /******************************************************************/
    /*! \brief Pointer-dereference passthrough of the pdf function

      \param in = the incoming direction
      \param out = the exitant direction
      \param component = which reflectance component was sampled
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the pdf of samling the in-out direction conbination.
    *******************************************************************/   
    virtual Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      return (*this)->pdf(in, out, component, unit, mask);
    }
    
    
    /******************************************************************/
    /*! \brief Pointer-dereference passthrough of the hemispherical reflectance function

      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = mask to enable/disable lanes
      \returns the approximate hemispherical reflectance (Spectrum) of the BSDF
    *******************************************************************/     
    virtual Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const override final
    {
      return (*this)->reflectance(out, component, unit, mask);
    }


    /*******************************************************************/
    /*! \brief Pointer-dereference passthrough of toString
     *******************************************************************/
    virtual std::string toString(void) const override final
    {
      return (*this)->toString();
    }

    /*******************************************************************/
    /*! \name Parameter Enumeration passthrough
     *******************************************************************/
    virtual bbm::vector<Value&> parameter_values(bsdf_attr flags=bsdf_attr::All) override final
    {
      return (*this)->parameter_values(flags);
    }
      
    virtual bbm::vector<const Value&> parameter_values(bsdf_attr flags=bsdf_attr::All) const override final
    {
      return (*this)->parameter_values(flags);
    }
      
    virtual bbm::vector<Value> parameter_default_values(bsdf_attr flags=bsdf_attr::All) const override final
    {
      return (*this)->parameter_default_values(flags);
    }
      
    virtual bbm::vector<Value> parameter_lower_bound(bsdf_attr flags=bsdf_attr::All) const override final
    {
      return (*this)->parameter_lower_bound(flags);
    }
      
    virtual bbm::vector<Value> parameter_upper_bound(bsdf_attr flags=bsdf_attr::All) const override final
    {
      return (*this)->parameter_upper_bound(flags);
    }
    //! @}
    
  private:
    //! \brief shared_prt to the actual BSDF implementation.
    pointer<bsdf_base<Config>> _bsdf;
  };

  BBM_CHECK_CONCEPT(concepts::bsdf_ptr, bsdf_ptr<config>);
  
  
  /*********************************************************************/
  /*! \brief Helper method for making a bsdf_ptr from a BSDF (new construction)

    \tparam BSDFTYPE = BSDF type that the ptr will point to.
    \param args = constuctor arguments of the BSDF type.

    This method allocates a new BSDF of BSDFTYPE, and constructs it with
    the given arguments.  The bsdf_ptr owns the BSDF type object.
  **********************************************************************/
  template<typename BSDFTYPE, typename... ARGS> requires concepts::bsdf<BSDFTYPE>
    bsdf_ptr< get_config<BSDFTYPE> > make_bsdf_ptr(ARGS... args)
  {
    auto ptr = std::make_shared<BSDFTYPE>(args...);
    return bsdf_ptr< get_config<BSDFTYPE> >(ptr);
  }
  
  /*********************************************************************/
  /*! \brief Helper method for making a bsdf_ptr from a BSDF MODEL (new construction)

    \tparam BSDFMODEL = BSDF model that the ptr will point to.
    \param args = constuctor arguments of the BSDF type.

    This method allocates a new BSDF of bsdf<BSDFMODEL>, and constructs it
    with the given arguments.  The bsdf_ptr owns the BSDF type object.
  **********************************************************************/
  template<typename BSDFMODEL, typename... ARGS> requires (concepts::bsdfmodel<BSDFMODEL> && !concepts::bsdf<BSDFMODEL>)
    bsdf_ptr< get_config<BSDFMODEL> > make_bsdf_ptr(ARGS... args)
  {
      auto ptr = std::make_shared<bsdf<BSDFMODEL>>(args...);
      return bsdf_ptr< get_config<BSDFMODEL> >(ptr);
  }
  
  /*********************************************************************/
  /*! \brief Helper method for making a bsdf_ptr from a BSDF (copy construction)

    \param arg = BSDF object we want to copy and have the bsdf_ptr point to.

    This method allocates a copy of the BSDFTYPE object (arg). The bsdf_ptr
    owns the copied object. 
  **********************************************************************/
  template<typename BSDFTYPE> requires (concepts::bsdf<BSDFTYPE> && !concepts::bsdf_ptr<BSDFTYPE>)
    bsdf_ptr< get_config<BSDFTYPE> > make_bsdf_ptr(const BSDFTYPE& arg)
  {
    auto ptr = std::make_shared<BSDFTYPE>(arg);
    return bsdf_ptr< get_config<BSDFTYPE> >(ptr);
  }

  /*********************************************************************/
  /*! \brief Helper method for making a bsdf_ptr from a BSDF MODEL (copy construction)

    \param arg = BSDF model object we want to copy and have the bsdf_ptr point to.

    This method allocates a bsdf<BSDFMODEL> object (copy from arg). The
    bsdf_ptr owns the copied object.
  **********************************************************************/
  template<typename BSDFMODEL> requires (concepts::bsdfmodel<BSDFMODEL> && !concepts::bsdf<BSDFMODEL>)
    bsdf_ptr< get_config<BSDFMODEL> > make_bsdf_ptr(const BSDFMODEL& arg)
  {
    auto ptr = std::make_shared<bsdf<BSDFMODEL>>(arg);
    return bsdf_ptr< get_config<BSDFMODEL> >(ptr);
  }

  /*********************************************************************/
  /*! \brief Helper method for making a bsdf_ptr (avoid bsdf_ptr of bsdf_ptr)

    \param arg = bsdf_ptr object we want to copy
  **********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    bsdf_ptr<CONF> make_bsdf_ptr(const bsdf_ptr<CONF>& arg)
  {
    return arg;
  }

} // end bbm namespace

#endif /* _BBM_BSDF_PTR_H_ */

       
