#ifndef _BBM_BSDF_ENUMERATE_H_
#define _BBM_BSDF_ENUMERATE_H_

#include <type_traits>

#include "core/attribute.h"
#include "core/enumerate.h"
#include "concepts/bsdf_attribute.h"

#include "util/flags.h"
#include "util/constfor.h"
#include "util/reflection.h"
#include "util/vector_util.h"

#include "bbm/bsdf_base.h"
#include "bbm/bsdfmodel.h"

/***********************************************************************/
/*! \file bsdf_enumerate.h
  \brief Enumerate all BSDF parameters (e.g., stack in a vector).
************************************************************************/

namespace bbm {

  /*** Implementation Details ***/
  namespace detail {

    //! \brief Enumerate all unnamed bsdf_attributes in a BSDF that meet the flag, and call the SELECT method on each and pass the result to CALLBACK.
    template<typename MODEL, typename CALLBACK, typename SELECT> requires concepts::bsdfmodel<MODEL>
      inline void enumerate_parameter_helper(bsdf_attr flag, MODEL&& model, CALLBACK&& callback, SELECT&& select)
    {
      // enumeration type
      using Value = std::conditional_t< bbm::is_const_v<MODEL>,
                                        const typename std::decay_t<MODEL>::Value&,
                                        typename std::decay_t<MODEL>::Value&
                                      >;
    
      // enumerate if it is an enumerable attribute that meets the flag
      CONSTFOR(idx, reflection::attributes_size<MODEL>,
      {
        auto&& val = std::get<idx>( attributes(std::forward<MODEL>(model)) );
        using val_t = decltype(val);

        if constexpr (concepts::bsdf_attribute<val_t> && is_enumerable_v<Value, val_t>)
            if( is_set(flag, std::decay_t<val_t>::prop::flag) )
              enumerate<Value>( select(std::forward<val_t>(val)), callback );
      });
      
      // Done.
    }

  } // end detail namespace

    
  /*********************************************************************/
  /*! \brief Enumerate the values of the attributes from a bsdf model
   *********************************************************************/
  template<typename MODEL, typename CALLBACK> requires concepts::bsdfmodel<MODEL>
    inline void enumerate_parameters(bsdf_attr flag, MODEL&& model, CALLBACK&& callback)
  {
    bbm::detail::enumerate_parameter_helper(flag, std::forward<MODEL>(model), callback, [](auto&& val) -> decltype(value(val)) { return value(val); } );
  }

  
  /*********************************************************************/
  /*! \brief Enumerate the default values of the attributes from a bsdf model
   *********************************************************************/
  template<typename MODEL, typename CALLBACK> requires concepts::bsdfmodel<MODEL>
    inline void enumerate_default_parameters(bsdf_attr flag, MODEL&& model, CALLBACK&& callback)
  {
    bbm::detail::enumerate_parameter_helper(flag, std::forward<MODEL>(model), callback, [](auto&& val) { return default_value(val); } );
  }

  
  /*********************************************************************/
  /*! \brief Enumerate the lower bounds of the attributes from a bsdf model
   *********************************************************************/
  template<typename MODEL, typename CALLBACK> requires concepts::bsdfmodel<MODEL>
    inline void enumerate_lower_bound(bsdf_attr flag, MODEL&& model, CALLBACK&& callback)
  {
    bbm::detail::enumerate_parameter_helper(flag, std::forward<MODEL>(model), callback, [](auto&& val) { return lower_bound(val); } );
  }


  /*********************************************************************/
  /*! \brief Enumerate the upper bounds of the attributes from a bsdf model
   *********************************************************************/
  template<typename MODEL, typename CALLBACK> requires concepts::bsdfmodel<MODEL>
    inline void enumerate_upper_bound(bsdf_attr flag, MODEL&& model, CALLBACK&& callback)
  {
    bbm::detail::enumerate_parameter_helper(flag, std::forward<MODEL>(model), callback, [](auto&& val) { return upper_bound(val); } );
  }


  /*********************************************************************/
  /*! \brief Enumerate the parameters of a BSDF model in a vector

    \param model = bsdf model to extract parameters from
    \param flag = bsdf_attr flag to select a subset of the parameters
    \returns vector of references to the bsdf parameter values
  **********************************************************************/
  template<typename MODEL> requires (concepts::bsdfmodel<MODEL> && !concepts::bsdf<MODEL>)
    inline auto parameter_values(MODEL&& model, bsdf_attr flag=bsdf_attr::All)
  {
    // value type
    using Value = std::conditional_t< bbm::is_const_v<MODEL>,
                                      const typename std::decay_t<MODEL>::Value&,
                                      typename std::decay_t<MODEL>::Value&
                                    >;

    // create vector
    bbm::vector<Value> result;
    enumerate_parameters(flag, std::forward<MODEL>(model), [&result](auto&& val) { result.push_back(std::forward<Value>(val)); });
    
    // Done.
    return result;
  }


  /*********************************************************************/
  /*! \brief Enumerate the parameters of a BSDF in a vector

    \param model = bsdf model to extract parameters from
    \param flag = bsdf_attr flag to select a subset of the parameters
    \returns vector of references to the bsdf parameter values
  **********************************************************************/
  template<typename BSDF> requires concepts::bsdf<BSDF>
    inline auto parameter_values(BSDF& bsdf, bsdf_attr flag=bsdf_attr::All)
  {
    return bsdf.parameter_values(flag);
  }

  
  /*********************************************************************/
  /*! \brief Enumerate the default parameters of a BSDF model in a vector

    \param model = bsdf model to extract default parameters from
    \param flag = bsdf_attr flag to select a subset of the parameters
    \returns vector of references to the bsdf parameter values
  **********************************************************************/
  template<typename MODEL> requires (concepts::bsdfmodel<MODEL> && !concepts::bsdf<MODEL>)
    inline auto parameter_default_values(MODEL&& model, bsdf_attr flag=bsdf_attr::All)
  {
    // value type
    using Value = typename std::decay_t<MODEL>::Value;

    // create vector
    bbm::vector<Value> result;
    enumerate_default_parameters(flag, std::forward<MODEL>(model), [&result](auto&& val) { result.push_back(std::forward<decltype(val)>(val)); });
    
    // Done.
    return result;
  }

  
  /*********************************************************************/
  /*! \brief Enumerate the default parameters of a BSDF in a vector

    \param model = bsdf model to extract default parameters from
    \param flag = bsdf_attr flag to select a subset of the parameters
    \returns vector of references to the bsdf parameter values
  **********************************************************************/
  template<typename BSDF> requires concepts::bsdf<BSDF>
    inline auto parameter_default_values(const BSDF& bsdf, bsdf_attr flag=bsdf_attr::All)
  {
    return bsdf.parameter_default_values(flag);
  }

  
  /*********************************************************************/
  /*! \brief Enumerate the lower bound of the parameters of a BSDF model in a vector

    \param model = bsdf model to extract lower bound from
    \param flag = bsdf_attr flag to select a subset of the parameters
    \returns vector of references to the bsdf parameter values
  **********************************************************************/
  template<typename MODEL> requires (concepts::bsdfmodel<MODEL> && !concepts::bsdf<MODEL>)
    inline auto parameter_lower_bound(MODEL&& model, bsdf_attr flag=bsdf_attr::All)
  {
    // value type
    using Value = typename std::decay_t<MODEL>::Value;

    // create vector
    bbm::vector<Value> result;
    enumerate_lower_bound(flag, std::forward<MODEL>(model), [&result](auto&& val) { result.push_back(std::forward<decltype(val)>(val)); });
    
    // Done.
    return result;
  }

  /*********************************************************************/
  /*! \brief Enumerate the lower bound of the parameters of a BSDF in a vector

    \param model = bsdf model to extract lower bound from
    \param flag = bsdf_attr flag to select a subset of the parameters
    \returns vector of references to the bsdf parameter values
  **********************************************************************/
  template<typename BSDF> requires concepts::bsdf<BSDF>
    inline auto parameter_lower_bound(const BSDF& bsdf, bsdf_attr flag=bsdf_attr::All)
  {
    return bsdf.parameter_lower_bound(flag);
  }


  /*********************************************************************/
  /*! \brief Enumerate the upper bound of the parameters of a BSDF model in a vector

    \param model = bsdf model to extract upper bound from
    \param flag = bsdf_attr flag to select a subset of the parameters
    \returns vector of references to the bsdf parameter values
  **********************************************************************/
  template<typename MODEL> requires (concepts::bsdfmodel<MODEL> && !concepts::bsdf<MODEL>)
    inline auto parameter_upper_bound(MODEL&& model, bsdf_attr flag=bsdf_attr::All)
  {
    // value type
    using Value = typename std::decay_t<MODEL>::Value;

    // create vector
    bbm::vector<Value> result;
    enumerate_upper_bound(flag, model, [&result](auto&& val) { result.push_back(std::forward<decltype(val)>(val)); });
    
    // Done.
    return result;
  }

  /*********************************************************************/
  /*! \brief Enumerate the upper bound of the parameters of a BSDF in a vector

    \param model = bsdf model to extract upper bound from
    \param flag = bsdf_attr flag to select a subset of the parameters
    \returns vector of references to the bsdf parameter values
  **********************************************************************/
  template<typename BSDF> requires concepts::bsdf<BSDF>
    inline auto parameter_upper_bound(const BSDF& bsdf, bsdf_attr flag=bsdf_attr::All)
  {
    return bsdf.parameter_upper_bound(flag);
  }

} // end bbm namespace

#endif /* _BBM_BSDF_ENUMERATE_H_ */
