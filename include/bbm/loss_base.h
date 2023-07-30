#ifndef _BBM_LOSS_BASE_H_
#define _BBM_LOSS_BASE_H_

#include "concepts/loss.h"

/************************************************************************/
/*! \file loss_base.h
  \brief Abstract base definition of a LOSS (with virtual functions)

  The difference between a lossfunction and a loss is that the lossfunction
  does not feature virtual functions, and thus the function call is decided at
  compile time.  loss_base on the other hand declares all methods as virtual,
  and thus it is decided at run-time which implementation to call.
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /* \brief Abstract base class of LOSSes (with virtual methods)

     Each LOSS implements the following __virtual__ methods:
     + update(void): initialize the loss computation. Should be called each time the optimization target is changed.
     + Value operator(Mask=true) const that returns the loss

     Implements: concepts::loss
  ***********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    struct loss_base
  {
    BBM_IMPORT_CONFIG( CONF );

    //! \brief empty virtual base destructor
    virtual ~loss_base(void) {}
    
    /********************************************************************/
    /*! \brief update

      Update the internal state of the loss computations; perform any
     intialization and/or precomputations needed.  This method should be called
     each time the optimization target changes.
    *********************************************************************/
    virtual void update(void) = 0;

    /********************************************************************/
    /*! \brief operator(): compute the loss

      \param mask = mask to enable/disable lanes.
      \returns the loss
    *********************************************************************/
    virtual Value operator()(Mask mask=true) const = 0;
  };
  
} // end bbm namespace

#endif /* _BBM_LOSS_BASE_H_ */
