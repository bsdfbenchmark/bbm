#ifndef _BBM_BSDF_SYMMETRY_H_
#define _BBM_BSDF_SYMMETRY_H_

/************************************************************************/
/*! \file bsdf_symmetry.h

  \brief Enum for passing Isotropic or Anisotropic options to a BSDF.

*************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief symmetry variants.

    symmetry_type = value type of isotropic and anisotropic roughness/sharpness
  **********************************************************************/
  enum struct symmetry_v
  {
    Isotropic = 0x0001,
    Anisotropic = 0x0002,
  };

  /**********************************************************************/
  /*! \brief Determine the data type for storing isotropic or anisotropic reflectance parameters.
   **********************************************************************/
  template<symmetry_v V, typename Value> 
    using symmetry_t = std::conditional_t<(V == symmetry_v::Isotropic), Value, vec2d<Value> >;

} // end bbm namespace

#endif /* _BBM_BSDF_SYMMETRY_H_ */
