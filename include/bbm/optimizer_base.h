#ifndef _BBM_OPTIMIZER_BASE_H_
#define _BBM_OPTIMIZER_BASE_H_

#include "concepts/optimizer.h"

/************************************************************************/
/*! \file optimizer.h
  \brief Abstract base definition of an optimizer (with virtual functions)

  The key difference between an optimization_algorithm and optimizer is that
  the former does not have virtual function, while the latters offers the same
  functionality but with virtual functions.
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Abstract optimizer base class

   **********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    struct optimizer_base
  {
    BBM_IMPORT_CONFIG( CONF );

    //! \brief empty virtual base destructor
    virtual ~optimizer_base(void) {}
    
    /********************************************************************/
    /*! \brief Perform one optimization step towards minimizing the loss.

      \returns the loss before taking an optimzation step
    *********************************************************************/
    virtual Value step(void) = 0;

    /********************************************************************/
    /*! \brief Reset the internal state of the optimizer
     ********************************************************************/
    virtual void reset(void) = 0;

    /********************************************************************/
    /*! \brief Check if the optimizer has converged
      *******************************************************************/
    virtual Mask is_converged(void) = 0;
  };
  
} // end bbm namespace

#endif /* _BBM_OPTIMIZER_BASE_H_ */
