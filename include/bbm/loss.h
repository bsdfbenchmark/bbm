#ifndef _BBM_LOSS_H_
#define _BBM_LOSS_H_

#include "concepts/loss.h"

#include "bbm/config.h"
#include "bbm/loss_base.h"

/************************************************************************/
/*! \file loss.h
  \brief Connects a lossfunction with a loss.

  This class provies a simple interface for connecting loss functions
  (without virtual functions) to a loss (with virtual functions). Essentially,
  this class just passes the method calls to the underlying loss function.

  Implements: concepts::loss
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief LOSS implementation of a loss function

    \tparam LOSSFUNCTION = the loss function to transfer into a LOSS
  ***********************************************************************/
  template<typename LOSSFUNCTION> requires concepts::lossfunction<LOSSFUNCTION>
    class loss : virtual public loss_base< get_config<LOSSFUNCTION> >, public LOSSFUNCTION
  {
  public:
    BBM_IMPORT_CONFIG( LOSSFUNCTION );

    //! \brief Inherit all constructors
    using LOSSFUNCTION::LOSSFUNCTION;

    //! \brief Construct directly from a LOSSFUNCTION
    loss(const LOSSFUNCTION& lossfunc) : LOSSFUNCTION(lossfunc) {}

    //! \brief Assignment operator
    using LOSSFUNCTION::operator=;

    /********************************************************************/
    /*! \brief Virtual passthrough of the update method
     ********************************************************************/
    virtual void update(void) override final
    {
      LOSSFUNCTION::update();
    }

    /********************************************************************/
    /*! \brief Virtual passthrough of the loss computation
     ********************************************************************/
    virtual Value operator()(Mask mask=true) const override final
    {
      return LOSSFUNCTION::operator()(mask);
    }
  };

  BBM_CHECK_CONCEPT(concepts::loss, loss<lossfunction<config>>);
  
} // end bbm namespace

#endif /* _BBM_LOSS_H_ */
