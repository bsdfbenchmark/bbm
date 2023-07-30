#ifndef _BBM_GRADIENT_H_
#define _BBM_GRADIENT_H_

#include "concepts/parameter.h"
#include "util/vector_util.h"

/***********************************************************************/
/*! \file gradient.h
  \brief Gradient related operations

  Enable gradients,detach, forward, backward, and get gradients types that
  fullfill concepts::diff_parameter
************************************************************************/

namespace bbm {
  
  /*********************************************************************/
  /*! \brief Enable gradients for a parameter set
   *********************************************************************/
  template<typename PARAM> requires concepts::diff_parameter<PARAM>
    inline void track_gradients(PARAM&& param, bool toggle=true)
  {
    for(auto& itr : param)
      track_gradient(itr, toggle);
  }

  
  /*********************************************************************/
  /*! @{ \brief Check if gradients are tracked
   *********************************************************************/
  template<typename PARAM> requires concepts::diff_parameter<PARAM>
    inline bool all_gradients_tracked(PARAM&& param)
  {
    for(auto& itr : param)
      if(!is_gradient_tracked(itr)) return false;
    return true;
  }

  template<typename PARAM> requires concepts::diff_parameter<PARAM>
    inline bool any_gradients_tracked(PARAM&& param)
  {
    for(auto& itr : param)
      if(is_gradient_tracked(itr)) return true;
    return false;
  }
  //! @}


  /*********************************************************************/
  /*! \brief Get the gradient from a parameter set
   *********************************************************************/
  template<typename PARAM> requires concepts::diff_parameter<PARAM>
    inline auto get_gradients(PARAM&& param)
  { 
    using Value = decltype( gradient(*std::begin(param)) );

    bbm::vector< std::decay_t<Value> > result;
    for(auto& itr : param)
      result.push_back( gradient(itr) );

    return result;
 }

  /*********************************************************************/
  /*! \brief Get the detached values from a parameter set
   *********************************************************************/
  template<typename PARAM> requires concepts::diff_parameter<PARAM>
    inline auto detach_gradients(PARAM&& param)
  {
    using Value = decltype( detach_gradient(*std::begin(param)) );

    bbm::vector< std::decay_t<Value> > result;
    for(auto& itr : param)
      result.push_back( detach_gradient(itr) );

    return result;
  }

    
  /*********************************************************************/
  /*! \brief Forward computation of gradients on a parameter set
   *********************************************************************/
  template<typename PARAM> requires concepts::diff_parameter<PARAM>
    inline void forward_gradients(PARAM&& param)
  {
    for(auto& itr : param)
      forward(itr);
  }

  /**********************************************************************/
  /*! \brief backward computations => passthrough to backbone
   **********************************************************************/
  template<typename T>
    inline void backward_gradients(T&& t)
  {
    backward(std::forward<T>(t));
  }
  
} // end bbm namespace

#endif /* _BBM_GRADIENT_H_ */
