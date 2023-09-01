Microfacet bsdfmodel
====================

A common type of bsdfmodel are microfacet models.  To ease the development of
microfacet models, and to maximize code reuse, BBM offers a microfacet bsdfmodel
specialization:

.. doxygenstruct:: bbm::microfacet

This bsdfmodel will be defined a `microfacet normal distribution`_, a `masking
and shadowing`_ implementation, a `Fresnel`_ implementation, a normalization
factor, and a name.  The normalization factor is provided by:

.. doxygenstruct:: bbm::microfacet_n
   :undoc-members:
   :members:
   :outline:
      
The default is ``microfacet_n::Unnormalized``. This ``microfacet_n::Cook``
corresponds to the normalization in the Cook-Torrance microfacet BRDF model,
i.e., a division by PI.  ``microfacet_n::Walter`` corresponds to the
additional normalization by 4 as proposed by Walter et al. in `Microfacet
Models for Refraction through Rough Surfaces
<http://dx.doi.org/10.2312/EGWR/EGSR07/195-206>`_.

Fresnel
-------

'Fresnel` implements Fresnel surface reflectance.  Each Fresnel implementation
must meet ``bbm::concepts::fresnel``:

.. doxygenconcept:: bbm::concepts::fresnel

``parameter_type`` are the fresnel parameter types from the ``bbm::ior``
namespace (i.e., ``bbm::ior::ior``, ``bbm::ior::reflectance``, etc...).

All Fresnel implementations reside in the ``bbm::fresnel`` namespace. Two
common Fresnel implementations are predefined:

.. doxygenstruct:: bbm::fresnel::cook
   :members:

.. doxygenstruct:: bbm::fresnel::schlick
   :members:

However, any implementation that follows `bbm::`concepts::fresnel`` is valid.
For example, `Bagher et
al. <https://doi.org/10.1111/j.1467-8659.2012.03147.x>`_ use a modified
Schlick Fresnel approximation that uses a 2D reflectance type.  Please refer
to `include/bsdfmodel/bagher.h <../doxygen/html/bagher_8h_source.html>`_ for
the implementation of the custom Fresnel method.

Microfacet Normal distribution
-------------------------------

The microfacet normal distribution, or NDF, describes the distribution of the
microfacets' normals which in the end determine the reflectance behavior of
the materials.  BBM provides an interface for NDF implementations.  Each NDF
must meet ``concepts::ndf``:

.. doxygenconcept:: bbm::concepts::ndf

.. note::

   NDFs are defined in the ``bbm::ndf`` namespace and stored in the
   ``include/ndf`` subdirectory.

We will demonstrate how to implement a new NDF by recreating the ``phong``
microfacet distribution (from Walter et al. in `Microfacet Models for
Refraction through Rough Surfaces
<http://dx.doi.org/10.2312/EGWR/EGSR07/195-206>`_).  We start by defining the
basic structure similar to a ``bsdfmodel``:

.. code-block:: c++

   #ifndef _BBM_PHONG_NDF_H_
   #define _BBM_PHONG_NDF_H_

   #include "bbm/ndf.h"

   namespace bbm {
     namespace ndf {

       template<typename CONF, string_literal NAME="Phong"> requires concepts::config<CONF>
         struct phong
       {
         BBM_IMPORT_CONFIG( CONF );
         static constexpr string_literal name = NAME;

         specular_sharpness<Value> sharpness;
         BBM_ATTRIBUTES(sharpness);

         BBM_DEFAULT_CONSTRUCTOR(phong) {}
       };
       
     } // end ndf namespace
   } // end bbm namespace
                

In this case, the ``ndf::phong`` implementation will feature one attribute:
``sharpness`` that we again expose via attribute reflection.  Similar as with
a ``bsdfmodel``, we let BBM automatically generate a constructor.

Next we add the four required functions:

.. code-block:: c++

   template<typename CONF, string_literal NAME="Phong"> requires concepts::config<CONF>
     struct phong
   {
     BBM_IMPORT_CONFIG( CONF );
     static constexpr string_literal name = NAME;

     Value eval(const Vec3d& halfway, Mask mask=true) const;
     Vec3d sample(const Vec3d& view, const Vec2d& xi, Mask mask=true) const;
     Value pdf(const Vec3d& view, const Vec3d& m, Mask mask=true) const;
     Value G1(const Vec3d& v, const Vec3d& m, Mask mask=true) const;
     
     specular_sharpness<Value> sharpness;
     BBM_ATTRIBUTES(sharpness);

     BBM_DEFAULT_CONSTRUCTOR(phong) {}
   };

   BBM_CHECK_CONCEPT(concepts::ndf, phong<config>);

In contrast to a ``bsdfmodel``, an ``ndf`` we opted not to support named
arguments for the four methods as the signatures of the methods are short and
they do not include many optional parameters. However, BBM does require named
arguments for the constructor.
                
The ``eval`` method evaluates the NDF given a ``halfway`` vector:

.. code-block:: c++

   Value eval(const Vec3d& halfway, Mask mask=true) const
   {
     // above surface?
     mask &= (vec::z(halfway) > 0);
      
     // Quick exit
     if(bbm::none(mask)) return 0;

     // eval NDF
     Value normalization = (sharpness + 2) / Constants::Pi(2);
     Value D = bbm::pow( spherical::cosTheta(halfway), sharpness ) * normalization;

     // Done.
     return bbm::select(mask, D, 0);
   }

The implementation is similar to that of a ``bsdfmodel``, except that we do
not need to check the light transport ``unit_t`` or ``component``.  Care must
be taken, to ensure that the implementation is compatible with both packet and
scalar types.

The ``sample`` method samples a new halfway vector based on two random values
(passed as a ``Vec2d``).  Additionally, a ``view`` vector is also passed to
support sampling methods that only consider visible microfacets.  This is
ignored in ``ndf::phong``:

.. code-block:: c++

   Vec3d sample(const Vec3d& /*view*/, const Vec2d& xi, Mask mask=true) const
   {
     // check valid xi
     mask &= (xi[0] >= 0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);

     // quick exit
     if(bbm::none(mask)) return 0;

     // sample microfacet normal
     Value cosTheta = bbm::pow( xi[0], 1.0 / (sharpness + 2) );
     Value sinTheta = bbm::safe_sqrt(1.0 - cosTheta*cosTheta);
     Vec2d csp = bbm::cossin( xi[1] * Constants::Pi(2) );
     
     // Done.
     return bbm::select(mask, vec::expand(csp*sinTheta, cosTheta), 0);
   }

In the ``sample`` method, we first check if the random values ``xi`` are valid
(i.e., between 0 and 1).  Next we compute the ``sin`` and ``cos`` of the theta
and phi angle of the sampled microfacet normal. Finally, we return the sampled
vector if the ``mask`` (including validity of the random variable) is true.
Note we abuse the joint computation of ``sin`` and ``cos`` with
``bbm::cossin`` which produces a ``Vec2d``, which we subsequently expand to a
``Vec3d`` with ``vec::expand``.

The ``pdf`` method returns the PDF corresponding to the sample method given a
microfacet normal ``m`` (and the ``view`` direction).  Unlike a ``bsdfmodel``,
the sample method of an ``ndf`` only returns the sampled microfacet normal,
not the PDF.

Finally, the ``G1`` method is the mono-directional shadowing and masking term
parameterized by the incident/outgoing vector ``v`` and the microfacet normal
``m``.  

Masking and shadowing
---------------------

The ndf's ``G1`` function is only models the mono-directional shadowing and
masking term.  Computing the bi-directional shadowing and masking
implementation.  Each ``maskingshadowing`` must meet
``bbm::concepts::maskingshadowing``:

.. doxygenconcept:: bbm::concepts::maskingshadowing

.. note::

   Maskingshadowing implementations are defined in the
   ``bbm::maskingshadowing`` namespace and the implementations are stored in
   ``include/maskingshadowing``.

A masking and shadowing implementation is a structure with a single static
method that takes the ``ndf``, in and out directions, and microfacet normal to
compute the shadowing and masking.  Four masking and shadowing methods have
been predefined:

.. doxygenstruct:: bbm::maskingshadowing::vgroove

.. doxygenstruct:: bbm::maskingshadowing::uncorrelated

.. doxygenstruct:: bbm::maskingshadowing::heightcorrelated

.. doxygenstruct:: bbm::maskingshadowing::vanginneken
   

Example: Cook-Torrance
----------------------

As an example of a microfacet bsdfmodel, consider the `Cook-Torrance microfacet
BRDF model <https://doi.org/10.1145/357290.357293>`_:

.. code-block:: c++

  template<typename CONF, string_literal NAME = "CookTorrance"> requires concepts::config<CONF> 
  using cooktorrance = scaledmodel<microfacet<ndf::beckmann<CONF,  symmetry_v::Isotropic, false>,
                                              maskingshadowing::vgroove<CONF>,
                                              fresnel::cook<CONF>,
                                              microfacet_n::Cook,
                                              NAME>,
                                   bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, cooktorrance<config>);

In this case the model consists of an unnormalized (``false``) isotropic
(``symmetry_v::Isotropic``) ``ndf::beckmann`` distribution, a
``maskingshadowing::vgroove`` and the ``fresnel::cook`` functions.  Because an
ndf is typically normalized, and thus does not contain an 'albedo' factor, we
wrap the microfacet bsdfmodel in a ``scaledmodel`` which is a ``bsdfmodel`` by
itself. ``scaledmodel`` passes through ``sample`` and ``pdf`` to the
underlying models, and scales the results of ``eval`` and ``reflectance`` by
an additional ``albedo`` attribute:

.. doxygenstruct:: bbm::scaledmodel
   :members:

ndf::sampler
------------

Not all NDF models have a published importance sampling formula.  BBM provides
a convenient numerical ``ndf::sampler`` for *isotropic* NDFs that constructs a
cumulative distribution function of the NDF, and numerically samples this:

.. doxygenclass:: bbm::ndf::sampler

For non-microfacet models, a similar numerical sampling approximation
exists. ``bbm::ndf_sampler`` wraps around an existing ``bsdfmodel`` and
constructs a numerical NDF by sampling the underlying ``bsdfmodel`` as if it
was a microfacet model (i.e., it samples halfway vectors (in == out) over the
hemisphere).  For example, the He et al. bsdf model has no known importance
sampling method.  BBM resolves this by wrapping the bsdfmodel implementation
(``he_base`` with a placeholder diffuse importance sampler) in an
ndf_sampler:

.. code-block:: c++

   template<typename CONF, string_literal NAME="He"> requires concepts::config<CONF>
    using he = ndf_sampler<he_base<CONF, ...>, 90, 1, NAME>;

Note for clarity we omit the various template arguments passed to ``he_base``.
