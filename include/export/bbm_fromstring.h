/************************************************************************/
/*! \file bbm_fromstring.h
  \brief fromString support for BBM

  Export bbm::fromString<BsdfMOdel> method calls to a named tuple with the
  name set to BsdfModel::name.  

  \param BsdfModel = the class name of the BSDF model to create a fromString handle for
  
  Assumes that:
  
  1. a bbm::fromString<BsdfModel> specialization exists for BsdfModel, either
     by using the default constructor (constructor_args_t is defined), or by a
     specialized static BsdfModel::fromString method.

  2. a BsdfModel::name static constexpr string_literal exists

  3. the BsdfModel takes a single template parameter that is the
  configuration.  Furthermore, we assume that a typedef Config exists.

  The macro will construct a named tuple with a single name (the
  BsdfModel::name), and a function pointer to the corresponding
  bbm::fromString<BsdfModel> specialization as value.

  Note: the creation of the named tuple is followed by a comma, not a semi
  comma. It is expected that all named tuples from the different bsdfmodel are
  then concat in a single tuple.  See bsdf_ptr for it usage.  Example:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  auto bsdf_fromString  = named_cat(
            #include "bbm_bsdfmodels.h"
            make_named<"dummy">(null_ptr)
            );

  auto Bsdf = bbm::get<"Lambertian">(bsdf_fromString)("Lambertian([1,1,1])");
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  This example will create a bsdf_fromString named tuple, where the names are
  the bsdfmodel names, and the values are function pointers to the
  corresponding fromString method. The second line passes a string to the
  fromString method that corresponds to the Lambertian bsdfmodel.

  Note: because each entry ends with a comma, we need to add one 'dummy' entry
  to close the list of named tuples.
  
*************************************************************************/

#undef BBM_EXPORT_BSDFMODEL
#define BBM_EXPORT_BSDFMODEL(BsdfModel) make_named<BsdfModel<Config>::name>( bbm::fromString<BsdfModel<Config>> ),
