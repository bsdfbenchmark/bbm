#ifndef _BBM_OPTIMIZER_H_
#define _BBM_OPTIMIZER_H_

#include "concepts/optimizer.h"

#include "bbm/config.h"
#include "bbm/optimizer_base.h"

/************************************************************************/
/*! \file optimizer.h
  \brief Connects a optimization_algorithm to a optimizers

  This class provides a simple interface for connecting optimization
  algorithms (without virtual functions) to a optimzier (with virtual
  functions).  Essentially, this class just passes the method calls to the
  underlying optimzation algorithm implementation.
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief OPTIMIZER implementation of a optimization algorithm

    \tparam OPT = the optimization algorithm
  ***********************************************************************/
  template<typename OPT> requires concepts::optimization_algorithm<OPT>
    class optimizer : virtual public optimizer_base< get_config<OPT> >, public OPT
  {
  public:
    BBM_IMPORT_CONFIG( OPT );

    //! \brief Inherit all constructors
    using OPT::OPT;

    //! \brief Directly construct from a OPTimization algorithm
    optimizer(const OPT& opt) : OPT(opt) {}

    //! \brief Assignment operartor
    using OPT::operator=;

    /********************************************************************/
    /*! \brief Virtual passthrough of the 'step' method

      \returns the current loss
    *********************************************************************/
    virtual Value step(void) override final
    {
      return OPT::step();
    }

    /********************************************************************/
    /*! \brief Virtual passthrough of the 'reset' method
     ********************************************************************/
    virtual void reset(void) override final
    {
      OPT::reset();
    }

    /********************************************************************/
    /*! \brief Virtual passthrough of the 'is_converged' method
     ********************************************************************/
    virtual Mask is_converged(void) override final
    {
      return OPT::is_converged();
    }
  };

} // end bbm namespace

BBM_CHECK_CONCEPT( concepts::optimizer, optimizer<optimization_algorithm<>> );

#endif /* _BBM_OPTIMIZER_H_ */
