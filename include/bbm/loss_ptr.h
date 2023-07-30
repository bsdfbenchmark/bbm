#ifndef _BBM_LOSS_PTR_H_
#define _BBM_LOSS_PTR_H_

#include "concepts/loss_ptr.h"
#include "concepts/macro.h"

#include "core/error.h"

#include "bbm/loss.h"

/************************************************************************/
/*! \file loss_ptr.h
  \brief A shared_ptr wrapper for losses.

  Implements: concepts::loss_ptr
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief loss_ptr: a wrapper around a shared_ptr to a loss

    \tparam CONF = bbm configuration
  ***********************************************************************/
  template<typename CONF>
    class loss_ptr : public virtual loss_base<CONF>
  {
  public:
    BBM_IMPORT_CONFIG( CONF );

    //! \brief Default (empty) constructor; shared_ptr is set to nullptr
    loss_ptr(void) = default;

    //! \brief Construct from shared_ptr
    template<typename LOSSTYPE> requires concepts::loss<LOSSTYPE>
      loss_ptr(const std::shared_ptr<LOSSTYPE>& ptr) : _loss(ptr) {}

    //! \brief Copy constructor
    loss_ptr(const loss_ptr& src) : _loss(src._loss) {}
    
    //! \brief Assignment operator
    loss_ptr& operator=(const loss_ptr& src)
    {
      _loss = src._loss;
      return *this;
    }

    //! \brief Get the interal shared pointer
    inline const std::shared_ptr<loss_base<Config>>& ptr(void) const { return _loss; }

    //! @{ \name Override the -> operator
    inline const std::shared_ptr<loss_base<Config>>& operator->(void)
    {
      if(!_loss) throw bbm_incomplete_init;
      return _loss;
    }

    inline std::shared_ptr<const loss_base<Config>> operator->(void) const
    {
      if(!_loss) throw bbm_incomplete_init;
      return _loss;
    }
    //! @}

    /********************************************************************/
    /*! \brief Pointer-dereference passthrough of the update method
     ********************************************************************/
    virtual void update(void) override final
    {
      (*this)->update();
    }

    /********************************************************************/
    /*! \brief Pointer-dereference passthrough of the loss evaluation operator()

      \param mask = mask to enable/disable lanes
      \returns the loss
    *********************************************************************/
    virtual Value operator()(Mask mask=true) const override final
    {
      return (*this)->operator()(mask);
    }

  private:
    /////////////////
    // Data Members
    /////////////////
    std::shared_ptr<loss_base<Config>> _loss;
  };

  BBM_CHECK_CONCEPT(concepts::loss_ptr, loss_ptr<config>);

  
  /**********************************************************************/
  /*! \brief Helper method for making loss_ptr from a LOSS (new construction)

    \tparam LOSSTYPE = LOSS type that the pointer will point to.
    \param args = constructor arguments for the LOSS type consructor

    This method allocates a new LOSS of LOSSTYPE, and constructs it with
    the given arguments. The loss_ptr owns the created object.
  ***********************************************************************/
  template<typename LOSSTYPE, typename... ARGS> requires concepts::loss<LOSSTYPE>
    loss_ptr< get_config<LOSSTYPE> > make_loss_ptr(ARGS&&... args)
  {
    auto ptr = std::make_shared<LOSSTYPE>(std::forward<ARGS>(args)...);
    return loss_ptr< get_config<LOSSTYPE> >(ptr);
  }

  /**********************************************************************/
  /*! \brief Helper method for making a loss_ptr from a LOSS FUNCTION (new construction)

    \tparam LOSSFUNCTION = loss function to encapsulate
    \param args = constructor arguments

    Allocates a new loss<LOSSFUNCTION> and constructs it with the given
    args.  The bsdf_ptr owns the BSDF object.
  ***********************************************************************/
  template<typename LOSSFUNC, typename... ARGS> requires (concepts::lossfunction<LOSSFUNC> && !concepts::loss<LOSSFUNC>)
    loss_ptr< get_config<LOSSFUNC> > make_loss_ptr(ARGS&&... args)
  {
    auto ptr = std::make_shared<loss<LOSSFUNC>>(std::forward<ARGS>(args)...);
    return loss_ptr< get_config<LOSSFUNC> >(ptr);
  }
  
  /**********************************************************************/
  /*! \brief Helper method for making a loss_ptr from a LOSS (copy construction)

    \param arg = LOSS object we want to copy

    A copy of the loss is allocated.  The loss_ptr owns the copied object
  ***********************************************************************/
  template<typename LOSSTYPE> requires (concepts::loss<LOSSTYPE> && !concepts::loss_ptr<LOSSTYPE>)
    loss_ptr< get_config<LOSSTYPE> > make_loss_ptr(const LOSSTYPE& arg)
  {
    auto ptr = std::make_shared<LOSSTYPE>(arg);
    return loss_ptr< get_config<LOSSTYPE> >(ptr);
  }

  /**********************************************************************/
  /*! \brief Helper method for making a loss_ptr from a LOSS FUNCTION (copy constructor)

    \param arg = LOSS FUNCTION object we want to copy

    A copy of the loss<LOSSFUNCTION> is allocated (copied from arg). The loss_ptr
    owns the copied object
  ***********************************************************************/
  template<typename LOSSFUNC> requires (concepts::lossfunction<LOSSFUNC> && !concepts::loss<LOSSFUNC>)
    loss_ptr< get_config<LOSSFUNC> > make_loss_ptr(const LOSSFUNC& arg)
  {
    auto ptr = std::make_shared<loss<LOSSFUNC>>(arg);
    return loss_ptr< get_config<LOSSFUNC> >(ptr);
  }

  /**********************************************************************/
  /*! \brief Helper method for making loss_ptr (avoid loss_ptr of loss_ptr)

    \param arg = loss_ptr object we want to copy
  ***********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    loss_ptr<CONF> make_loss_ptr(const loss_ptr<CONF>& arg)
  {
    return arg;
  }
  
} // end bbm namespace

#endif /* _BBM_LOSS_PTR_H_ */
