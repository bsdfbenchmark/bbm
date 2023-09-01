#ifndef _BBM_CONFIG_H_
#define _BBM_CONFIG_H_

#include "concepts/macro.h"
#include "concepts/config.h"

#include "core/vec.h"
#include "core/color.h"
#include "core/mat.h"
#include "core/constants.h"
#include "core/stringconvert.h"

#include "bbm/bsdf_flag.h"

/*************************************************************************/
/*! \file config.h

  \brief All BBM methods are defined to operate on a variety of value types
    and spectrum types.  Each BBM class takes a config structure as a template
    parameter (that satisfies concepts::config).  To import common
    typedefs into the current scope add the macro BBM_IMPORT_CONFIG with the
    config template parameters as parameter.  To import a single 'core' type
    use Core_t<Config>.
**************************************************************************/

namespace bbm {

  /***********************************************************************/
  /*! \brief Helper Macro to validate a configuration
   ***********************************************************************/
  #define BBM_CHECK_CONFIG(CONF)                                          \
    BBM_CHECK_RAW_CONCEPT( bbm::concepts::config, CONF);                  \
    BBM_VALIDATE_BACKBONE(typename CONF::Value);                          \
    BBM_VALIDATE_BACKBONE(typename CONF::Spectrum);                       \
    BBM_VALIDATE_BACKBONE( bbm::vec2d<typename CONF::Value> );            \
    BBM_VALIDATE_BACKBONE( bbm::vec3d<typename CONF::Value> );            \
    BBM_VALIDATE_BACKBONE( bbm::complex<typename CONF::Value> );          \
    BBM_CHECK_RAW_CONCEPT( std::constructible_from, bbm::vec2d<typename CONF::Value>, typename CONF::Value, typename CONF::Value ); \
    BBM_CHECK_RAW_CONCEPT( std::constructible_from, bbm::vec3d<typename CONF::Value>, typename CONF::Value, typename CONF::Value, typename CONF::Value ); \
    BBM_CHECK_RAW_CONCEPT( bbm::concepts::backbone::has_complex_functions, bbm::complex<typename CONF::Value> ); \

  
  /**********************************************************************/
  /*! \brief get_config type trait

    Get the config struct type
   **********************************************************************/
  template<typename T> requires concepts::has_config<T>
    using get_config = typename std::decay_t<T>::Config;

  
  /*** Forward Declarations for Config dependent types included in BBM_IMPORT_CONFIG ***/
  #define BBM_DECLARE_FORWARD_IMPORT(STRUCT)  template<typename CONF> requires concepts::config<CONF> struct STRUCT;
  BBM_DECLARE_FORWARD_IMPORT(bsdfsample);
  BBM_DECLARE_FORWARD_IMPORT(vec3dpair);

  /*** Determine a 'core' type from a config; checks if the default needs to be overriden; use as Core_t<T> ***/
  #define BBM_DECLARE_CORE_TYPE(Core, Type, ...)                        \
    namespace detail {                                                   \
      template<typename Type> struct Core { using type = __VA_ARGS__; }; \
      template<typename Type> requires requires { typename std::decay_t<Type>::Core; } \
        struct Core<Type> { using type = typename std::decay_t<Type>::Core; }; \
    }                                                                    \
    template<typename T> requires bbm::concepts::has_config<T>           \
      using Core ## _t = typename bbm::detail::Core<get_config<T>>::type;\

  template<typename T> requires concepts::has_config<T> using Value_t = typename get_config<T>::Value;
  template<typename T> requires concepts::has_config<T> using Spectrum_t = typename get_config<T>::Spectrum;

  BBM_DECLARE_CORE_TYPE(Scalar, T, bbm::scalar_t<Value_t<T>>);
  BBM_DECLARE_CORE_TYPE(Mask, T, bbm::mask_t<Value_t<T>>);
  BBM_DECLARE_CORE_TYPE(Size, T, bbm::index_t<Value_t<T>>);
  BBM_DECLARE_CORE_TYPE(BsdfFlag, T, bbm::replace_scalar_t<bbm::remove_diff_t<Value_t<T>>, bbm::bsdf_flag>);

  BBM_DECLARE_CORE_TYPE(Constants, T, bbm::constants<Scalar_t<T>>);
  BBM_DECLARE_CORE_TYPE(Vec2d, T, bbm::vec2d<Value_t<T>>);
  BBM_DECLARE_CORE_TYPE(Mat2d, T, bbm::mat2d<Value_t<T>>);
  BBM_DECLARE_CORE_TYPE(Vec3d, T, bbm::vec3d<Value_t<T>>);
  BBM_DECLARE_CORE_TYPE(Mat3d, T, bbm::mat3d<Value_t<T>>);
  BBM_DECLARE_CORE_TYPE(Complex, T, bbm::complex<Value_t<T>>);

  /*** Define short hand defintions similar to Core_t<T> for selected non-Core types ***/
  #define BBM_DECLARE_SHORTHAND(SHORTHAND, TYPE)                          \
    template<typename T> requires bbm::concepts::has_config<T>            \
      using SHORTHAND = TYPE<bbm::get_config<T>>;                         \
    
  BBM_DECLARE_SHORTHAND(BsdfSample_t, bsdfsample);
  BBM_DECLARE_SHORTHAND(Vec3dPair_t, vec3dpair);
  
  /***********************************************************************/
  /*! \brief Import the configs typedefs in the current scope          

    A macro to define common shorthand type aliases in the current scope
    based on a BBM config structure type.

    Any class that meets concepts::has_config can be passed.

    Using __VA_ARGS__ for passing the CONFIG argument to handle passing
    a configs with commas.

    Any class that IMPORTs a CONFIG will satisfies concepts::has_config.
  ***********************************************************************/
#define BBM_IMPORT_CONFIG(...)                                           \
    using Config              = bbm::get_config<__VA_ARGS__>;            \
    BBM_CHECK_CONFIG( Config );                                          \
    BBM_BACKBONE_IMPORT( Config );                                       \
                                                                         \
    /*                 Core types                                      */\
    using Value               = bbm::Value_t<Config>;                    \
    using Spectrum            = bbm::Spectrum_t<Config>;                 \
    using Scalar              = bbm::Scalar_t<Config>;                   \
    using Mask                = bbm::Mask_t<Config>;                     \
    using Size_t              = bbm::Size_t<Config>;                     \
    using BsdfFlag            = bbm::BsdfFlag_t<Config>;                 \
                                                                         \
    using Constants           = bbm::Constants_t<Config>;                \
    using Vec2d               = bbm::Vec2d_t<Config>;                    \
    using Mat2d               = bbm::Mat2d_t<Config>;                    \
    using Vec3d               = bbm::Vec3d_t<Config>;                    \
    using Mat3d               = bbm::Mat3d_t<Config>;                    \
    using Complex             = bbm::Complex_t<Config>;                  \
                                                                         \
    /*                Convenient Shorthands                            */\
    using BsdfSample          = bbm::bsdfsample<Config>;                 \
    using Vec3dPair           = bbm::vec3dpair<Config>;                  \

} // end bbm namespace

//////////////////////////////////////////////////////////////
// Import Config Dependent Classes used in BBM_IMPORT_CONFIG
//////////////////////////////////////////////////////////////
#include "bbm/bsdfsample.h"
#include "bbm/vec3dpair.h"

#endif /* _BBM_CONFIG_H_ */
