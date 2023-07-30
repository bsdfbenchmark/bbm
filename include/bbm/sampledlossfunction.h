#ifndef _BBM_SAMPLEDLOSSFUNCTION_H_
#define _BBM_SAMPLEDLOSSFUNCTION_H_

#include "concepts/samplelossfunction.h"
#include "concepts/sampledlossfunction.h"
#include "concepts/inout_linearizer.h"
#include "concepts/bsdfmodel.h"

#include "bbm/config.h"
#include "util/reference.h"

/***********************************************************************/
/*! \file sampledlossfunction.h

  \brief Definition of a loss function that is the sum of losses on samples.

************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief sampledlossfunction

    Computes the loss over BSDFs sampled by a linearizer.

    Satisfied: concepts::sampledlossfunction
  **********************************************************************/
  template<typename BSDF, typename REFERENCE, typename SAMPLELOSSFUNC, typename LINEARIZER, bsdf_flag COMPONENT=bsdf_flag::All, unit_t UNIT=unit_t::Radiance>
    requires concepts::bsdfmodel<BSDF> &&
             concepts::bsdfmodel<REFERENCE> &&
             concepts::inout_linearizer<LINEARIZER> &&
             concepts::samplelossfunction<SAMPLELOSSFUNC> &&
             concepts::matching_config<BSDF, REFERENCE, LINEARIZER>
    struct sampledlossfunction
  {
    BBM_IMPORT_CONFIG( BSDF );

    /*******************************************************************/
    /*! \brief Constructor

      \param bsdf = bsdf model to be optimized
      \param reference = goal bsdf model
      \param samplelossfunc = loss over a single loss sample
      \param linearizer = linearizer to sample the bsdf models.
      
    ********************************************************************/
    inline sampledlossfunction(const BSDF& bsdf, const REFERENCE& reference, const SAMPLELOSSFUNC& samplelossfunc, const LINEARIZER& linearizer) : _bsdf(bsdf), _reference(reference), _linearizer(linearizer), _samplelossfunc(samplelossfunc) {}

    /*******************************************************************/
    /*! \brief Init (does nothing)
     *******************************************************************/
    inline void update(void) {}
    
    /*******************************************************************/
    /*! \brief Returns the number of samples
     *******************************************************************/
    inline Size_t samples(void) const { return _linearizer.size(); }

    /*******************************************************************/
    /*! \brief Compute the loss over the idx-th sample
     *******************************************************************/
    inline Value operator()(Size_t idx, Mask mask=true) const
    {
      // check bounds
      mask &= (idx < samples());
      if(bbm::none(mask)) return 0;
      
      Vec3dPair directions = _linearizer(idx);
      return _samplelossfunc(directions.in, directions.out,
                             _bsdf.eval(directions.in, directions.out, COMPONENT, UNIT, mask),
                             _reference.eval(directions.in, directions.out, COMPONENT, UNIT, mask)
                             );
    }

    /*******************************************************************/
    /*! \brief Compute loss over all samples
     *******************************************************************/
    inline Value operator()(Mask mask=true) const
    {
      Value err(0);
      Size_t numsamples = samples();
      
      for(size_t i=0; i < bbm::hmax(numsamples); ++i)
        err += operator()(i, mask && (i < numsamples));
        
      return err / Value(numsamples);
    }
    
  private:
    /////////////////////
    // Class Attributes
    /////////////////////
    const BSDF& _bsdf;
    const REFERENCE& _reference;
    LINEARIZER _linearizer;
    SAMPLELOSSFUNC _samplelossfunc;
  };

  BBM_CHECK_CONCEPT( concepts::sampledlossfunction, bbm::sampledlossfunction<bsdfmodel<>, bsdfmodel<>, samplelossfunction<>, inout_linearizer<>> );
  
} // end bbm namespace

#endif /* _BBM_SAMPLEDLOSSFUNCTION_H_ */
