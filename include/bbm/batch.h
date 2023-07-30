#ifndef _BBM_BATCH_H_
#define _BBM_BATCH_H_

#include "concepts/sampledlossfunction.h"
#include "bbm/config.h"
#include "util/vector_util.h"

/************************************************************************/
/*! \file batch.h

  \brief Batch loss wrapper for sampled loss functions.

*************************************************************************/

namespace bbm {

  
  /*********************************************************************/
  /* \brief batch; a specialized sampled loss function

     Computes the loss over a sampled loss for a given batch size of
     randomly picked samples.

     Satisfies: concepts::sampledlossfunction
  **********************************************************************/
  template<typename SAMPLEDLOSSFUNC> requires concepts::sampledlossfunction<SAMPLEDLOSSFUNC>
    struct batch
  {
    BBM_IMPORT_CONFIG( SAMPLEDLOSSFUNC );

    /********************************************************************/
    /*! \brief Constructor

      Wrapper around sampled loss functions that only evaluates the loss
      for a subset of the samples.  Each update re-randomizes the subset.
      
      \param batchsize = size of the batch (same batch size for all packets)
      \param sampedlosfunc = any sampled loss function (to be subsampled).
    *********************************************************************/
    inline batch(size_t batchsize, const SAMPLEDLOSSFUNC& sampledlossfunc, seed_t seed=default_seed) : _sampledlossfunc(sampledlossfunc), _index(batchsize), _rng(seed, 0, sampledlossfunc.samples())
    {
      update();
    }

    /*******************************************************************/
    /*! \brief Init the list of random indices.
     *******************************************************************/
    inline void update(void)
    {
      _sampledlossfunc.update();
      
      for(size_t i=0; i < _index.size(); ++i)
        _index[i] = _rng();
    }
    
    /*******************************************************************/
    /*! \brief Returns the number of samples
     *******************************************************************/
    inline Size_t samples(void) const { return _index.size(); }
    
    /*******************************************************************/
    /*! \brief Compute the loss over the I-th sample
     *******************************************************************/
    inline Value operator()(Size_t idx, Mask mask=true) const
    {
      mask &= (idx < samples());
      if(bbm::none(mask)) return 0;
      
      return _sampledlossfunc(_index[idx], mask);
    }

    /*******************************************************************/
    /*! \brief Compute loss over all samples
     *******************************************************************/
    inline Value operator()(Mask mask=true) const
    {
      Value err(0);
      for(size_t i; i < _index.size(); ++i)
        err += operator()(i, mask);

      return err / samples();
    }
    
    
  private:
    /////////////////////
    // Class Attributes
    /////////////////////
    bbm::rng<Size_t> _rng;
    bbm::vector<Size_t> _index;
    SAMPLEDLOSSFUNC _sampledlossfunc;
  };

  BBM_CHECK_CONCEPT(concepts::sampledlossfunction, batch<concepts::archetype::sampledlossfunction<>>);

} // end bbm namespace

#endif /* _BBM_BATCH_H_ */
