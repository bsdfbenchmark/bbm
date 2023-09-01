#ifndef _BBM_OPTIMIZER_PTR_H_
#define _BBM_OPTIMIZER_PTR_H_

#include "concepts/optimizer_ptr.h"
#include "concepts/macro.h"

#include "core/error.h"

#include "bbm/config.h"
#include "bbm/optimizer.h"

/************************************************************************/
/*! \file optimizer_ptr.h
  \brief A shared_ptr wrapper for optimizers.

  Implements: concepts::optimizer_ptr
************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief optimizer_ptr: a wrapper around a shared_ptr to an optimizer

    \tparam CONF = bbm configuration
  ***********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    class optimizer_ptr : public virtual optimizer_base<CONF>
  {
  public:
    BBM_IMPORT_CONFIG( CONF );

    //! \brief Default (empty) constructor; shared_ptr is set to nullptr
    optimizer_ptr(void) = default;

    //! \brief Construct from shared_ptr
    template<typename OPT> requires concepts::optimizer<OPT>
      optimizer_ptr(const std::shared_ptr<OPT>& ptr) : _opt(ptr) {}

    //! \brief Assignment operator
    optimizer_ptr& operator=(const optimizer_ptr& src)
    {
      _opt = src._opt;
      return *this;
    }

    //! \brief Get the internal shared pointer
    inline const std::shared_ptr<optimizer_base<Config>>& ptr(void) const { return _opt; }

    //! @{ \name Override the -> operator
    inline const std::shared_ptr<optimizer_base<Config>>& operator->(void)
    {
      if(!_opt) throw bbm_incomplete_init;
      return _opt;
    }

    inline std::shared_ptr<optimizer_base<Config>> operator->(void) const
    {
      if(!_opt) throw bbm_incomplete_init;
      return _opt;
    }
    //! @}
    
    /********************************************************************/
    /*! \brief Dereference passthrough of the 'step' method

      \returns the current loss
    *********************************************************************/
    virtual Value step(void) override final
    {
      return (*this)->step();
    }

    /********************************************************************/
    /*! \brief Dereference passthrough of the 'reset' method
     ********************************************************************/
    virtual void reset(void) override final
    {
      (*this)->reset();
    }

    /********************************************************************/
    /*! \brief Dereference passthrough of the 'is_converged' method
      *******************************************************************/
    virtual Mask is_converged(void) override final
    {
      return (*this)->is_converged();
    }

  private:
    /////////////////
    // Data Members
    /////////////////
    std::shared_ptr<optimizer_base<Config>> _opt;
  };   

  BBM_CHECK_CONCEPT(concepts::optimizer_ptr, optimizer_ptr<config>);

  
  /**********************************************************************/
  /*! \brief Helper method for making an optimizer_ptr from an optimizer (new construction)

    \tparam OPTIMIZER = optimizer type that the pointer will point to.
    \param args = constructor arguments for the OPTIMIZER

    The method allocates a new OPTIMIZER and constructs it with the given
    arguments.  The optimizer_ptr owns the created object.
  ***********************************************************************/
  template<typename OPTIMIZER, typename... ARGS> requires concepts::optimizer<OPTIMIZER>
    optimizer_ptr<get_config<OPTIMIZER>> make_optimizer_ptr(ARGS&&... args)
  {
    auto ptr = std::make_shared<OPTIMIZER>(std::forward<ARGS>(args)...);
    return optimizer_ptr<get_config<OPTIMIZER>>(ptr);
  }

  /**********************************************************************/
  /*! \brief Helper method for making an optimizer_ptr from a OPTIMIZATION_ALGORITHM

    \tparam OPTIMIZATION_ALGORITHM = algorithm to point to
    \param args = constructor arguments

    Allocates a new optmizer<OPTIMIZATION_ALGORITHM> and constructs it with the
    given args. The optimizer_ptr owns the OPTIMIZER object.
  ***********************************************************************/
  template<typename OPTIMIZATION_ALGORITHM, typename... ARGS> requires (concepts::optimization_algorithm<OPTIMIZATION_ALGORITHM> && !concepts::optimizer<OPTIMIZATION_ALGORITHM>)
    optimizer_ptr<get_config<OPTIMIZATION_ALGORITHM>> make_optimizer_ptr(ARGS&&... args)
  {
    auto ptr = std::make_shared<optimizer<OPTIMIZATION_ALGORITHM>>(std::forward<ARGS>(args)...);
    return optimizer_ptr<get_config<OPTIMIZATION_ALGORITHM>>(ptr);
  }

  /**********************************************************************/
  /*! \brief Helper method for making an optimizer_ptr from a OPTIMZER (copy construction)

    \param arg = optimizer to copy

    A copy of the optimizer is allocated. The optimizer_ptr owns the copied object
  ***********************************************************************/
  template<typename OPTIMIZER> requires concepts::optimizer<OPTIMIZER>
    optimizer_ptr<get_config<OPTIMIZER>> make_optimizer_ptr(const OPTIMIZER& arg)
  {
    auto ptr = std::make_shared<OPTIMIZER>(arg);
    return optimizer_ptr<get_config<OPTIMIZER>>(ptr);
  }

  /**********************************************************************/
  /*! \brief Helper method for making an optimizer_ptr from a OPTIMIZATION_ALGORITHM (copy construction)

    \param arg = optimization algorithm to copy

    A copy of the optimizer<OPTIMIZATION_ALGORITHM> is allocated (copied from
    arg). The optimizer_ptr owns the copied object.
  ************************************************************************/
  template<typename OPTIMIZATION_ALGORITHM> requires (concepts::optimization_algorithm<OPTIMIZATION_ALGORITHM> && !concepts::optimizer<OPTIMIZATION_ALGORITHM>)
    optimizer_ptr<get_config<OPTIMIZATION_ALGORITHM>> make_optimizer_ptr(const OPTIMIZATION_ALGORITHM& arg)
  {
    auto ptr = std::make_shared<optimizer<OPTIMIZATION_ALGORITHM>>(arg);
    return optimizer_ptr<get_config<OPTIMIZATION_ALGORITHM>>(ptr);
  }

  /**********************************************************************/
  /*! \brief Helper method for making an optimizer_ptr (avoid optimizer_ptr of optimizer_ptr>)

    \param arg = optimizer_ptr object we want to copy
  **********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    optimizer_ptr<CONF> make_optimizer_ptr(const optimizer_ptr<CONF>& arg)
  {
    return arg;
  }
  
} // end bbm namespace

#endif /* _BBM_OPTIMIZER_PTR_H_ */
