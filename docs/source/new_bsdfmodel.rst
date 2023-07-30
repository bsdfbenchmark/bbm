Implementing a bsdfmodel
========================

As an example, we will reimplement the ``lambertian`` bsdfmodel step by
step. We start with the empty struct:

.. code-block:: c++

   #ifndef _BBM_LAMBERTIAN_H_
   #define _BBM_LAMBERTIAN_H_

   #include "bbm/bsdfmodel.h"

   namespace bbm {

     template<typename CONF, string_literal NAME="Lambertian"> requires concepts::config<CONF>
       struct lambertian
     {
       BBM_IMPORT_CONFIG( CONF );
       static constexpr string_literal name = NAME;
     };
   
   } // end bbm namespace

   #endif /* _BBM_LAMBERTIAN_H_ */

.. note::

   A ``bsdfmodel`` is typically a header file only implementation stored in the
   ``include/bsdfmodel`` subdirectory.  A static or measured ``bsdfmodel`` is
   stored in the ``include/staticmodel`` subdirectory.
   
All ``bsdfmodel`` structs are placed in the ``bbm`` namespace, and by
convention, each ``bsdfmodel`` takes a bbm config as its first template
argument (we use ``concepts::config`` to ensure it is a valid config).  Most
``bsdfmodel`` structs also take a ``string_literal`` to set the name of the
object.  This name will be used for exporting the ``bsdfmodel`` and for
automatically creating a human-readable object-to-string.  We template the
bsdfmodel name to allow for specializations or aliases with different names.

At the top of the ``bsdfmodel`` we import the config (``BBM_IMPORT_CONFIG``)
and set a ``static constexpr string_literal name``.
   
.. note::

  BBM requires explicite define guards ``_BBM_LAMBERTIAN_H_`` instead of
  relying on ``#pragma`` commands, in order to support automated ``bsdfmodel``
  exporting (which requires a macro command to be placed *outside* the guards
  after the definition of the ``bsdfmodel``.

Next, we will add, at the end of the class the model attributes, and the
default constructor:

.. code-block:: c++

   template<typename CONF, string_literal NAME="Lambertian"> requires concepts::config<CONF>
     struct lambertian
   {
     BBM_IMPORT_CONFIG( CONF );
     static constexpr string_literal name = NAME;

     diffuse_scale<Spectrum> albedo;
     BBM_ATTRIBUTES(albedo);

     BBM_DEFAULT_CONSTRUCTOR(lambertian) {}
   };

Model attributes are specializations of a ``bsdf_parameter``:

.. doxygentypedef:: bbm::bsdf_parameter

Hence, each ``bsdf_parameter`` is an ``attribute`` with ``bsdf_properties``:

.. doxygenstruct:: bbm::bsdf_properties

Several ``bsdf_parameter`` types are already predefined:

.. doxygentypedef:: bbm::bsdf_scale

.. doxygentypedef:: bbm::bsdf_roughness

.. doxygentypedef:: bbm::bsdf_sharpness

Each with a corresponding diffuse and specular variant, e.g.:

.. doxygentypedef:: bbm::diffuse_scale

Finally, a predefined parameter type with appropriate bounds exists for
attributes used in the Fresnel equations:

.. doxygentypedef:: bbm::fresnel_parameter

where the valid types ``T`` are:

.. doxygentypedef:: bbm::ior::ior

.. doxygentypedef:: bbm::ior::reflectance

.. doxygentypedef:: bbm::ior::complex_ior

for modeling the index of refraction, surface reflectance at normal incidence,
and the complex index of refraction.

.. note::

   Fresnel parameters are defined in the ``bbm::ior`` namespace.  Each is
   defined by adding a `tag` to their base type.  For example, both
   ``ior::ior<float>`` and ``ior::reflectance<float>`` store their data as a
   float, but differ in their `tag`, namely ``bbm::ior::ior_tag<float>`` and
   ``bbm::ior::reflectance_tag<float>``.  From a programmers perspective, both
   behave like a regular ``float``, except that you can query with type_traits
   (``is_ior_v``, ``is_reflectance_v``, and ``is_complex_ior_v``) which type
   it is, as well as detect whether it is any Fresnel type (``is_ior_type``).
   When assigning ``bbm::ior::ior`` types to ``bbm::ior::reflectance``, or
   vice versa, the value is automatically converted.

To support easy access and abstraction of higher-level methods, BBM relies on
compile-time reflection.  Currently, BBM supports reflection of class
attributes and class-inheritance.  To add attribute reflection, we use the
macro:

.. code-block:: c++

   BBM_ATTRIBUTES( <attribute names> );

Leveraging reflection, BBM can also automatically create a default constructor
by invoking the macro:

.. code-block:: c++

   BBM_DEFAULT_CONSTRUCTOR( <class name> )
   {
     <constructor body>
   }

The default constructor includes:

* Set up the constructor arguments to reflect the attributes (in type *and* order);
* Set up the default value of the constructor arguments to their
  ``default_value`` as specified in by ``bsdf_parameter``;
* Add support for named constructor arguments (i.e., "argument_name"_arg =
  ...);
* Executes the `<constructor body>` after setting the values of the class
  attributes.  In most cases (as is in the example above) this body is empty;
* Set up a typedef ``constructor_args_t`` that describes the arguments of the
  constructor.  For example:
  
  .. code-block:: c++

     std::cout << bbm::typestring<lambertian<Config>::constructor_args_t> << std::endl;

  yields (cleaned up for readability):

  .. code-block:: none

     bbm::args<bbm::arg<const Spectrum&, bbm::string_literal{"albedo"}, <lambda()> > >

  where the lambda function sets the default argument value (``Spectrum(0.5,
  0.5, 0.5)`` in this case).

Next, we add the four required ``bsdfmodel`` methods:

.. code-block:: c++

   template<typename CONF, string_literal NAME="Lambertian"> requires concepts::config<CONF>
     struct lambertian
   {
     BBM_IMPORT_CONFIG( CONF );
     static constexpr string_literal name = NAME;
     BBM_BSDF_FORWARD;

     Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const;
     BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const;     
     Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const;
     Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const;

     diffuse_scale<Spectrum> albedo;
     BBM_ATTRIBUTES(albedo);

     BBM_DEFAULT_CONSTRUCTOR(lambertian) {}
   };

.. note::

   The macro ``BBM_BSDF_FORWARD`` extends the method calls to ``eval``,
   ``sample``, ``pdf``, and ``reflectance`` with named arguments.  This is
   particularly useful for example if one wants to keep ``unit`` and
   ``component`` at their default value, but wants to alter ``mask``:

   .. code-block:: c++

      model.eval(in, out, "mask"_arg = false);

   BBM's named arguments are smart enough to understand that there is only a
   single ``Mask`` type in the method call, and thus we can omit the name in
   this case:

   .. code-block:: c++

      model.eval(in, out, Mask(false));

   ``unit_t`` and ``BsdfFlag`` are unique types, and thus can benefit from the
   same shorthand notation.

   .. warning::

      The type have to match exactly (after decay). Hence, ``false`` should be
      cast to ``Mask`` to ensure it is the exact type.

Next, we add an implementations for each method:

.. code-block:: c++

   Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
   {
      // diffuse?
      mask &= is_set(component, bsdf_flag::Diffuse);
        
      // above surface?
      mask &= (vec::z(in) >= 0) && (vec::z(out) >= 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return Spectrum(0);
        
      // compute reflectance
      Spectrum result = albedo * Constants::InvPi();
      return bbm::select(mask, result, 0);
   }

First, we check if the ``component`` requested is diffuse (since the
lambertian model only models diffuse reflectance).  In the second line, we
check if both the ``in`` and ``out`` directions are above the surface.  Note
that we only in the 3rd line, return ``Spectrum(0)`` if any of the above tests
failed all *all* elements in the mask (which can be more than one in case of
packet types).  Finally, we compute the result, and return it.  We use
``bbm::select`` to set the result to ``0`` if the corresponding element in the
``mask`` is false.

Please refer to `include/bsdfmodel/lambertian.h
<../doxygen/html/lambertian_8h_source.html>`_ for details on the
implementations for ``sample``, ``pdf``, and ``reflectance``.

Finally, we add two more macro calls to complete the implementation:

.. code-block:: c++
                
   #ifndef _BBM_LAMBERTIAN_H_
   #define _BBM_LAMBERTIAN_H_

   #include "bbm/bsdfmodel.h"

   namespace bbm {

     template<typename CONF, string_literal NAME="Lambertian"> requires concepts::config<CONF>
       struct lambertian
     {
       /* ... */
     };

     BBM_CHECK_CONCEPT(concepts::bsdfmodel, lambertian<config>);
     
   } // end bbm namespace

   #endif /* _BBM_LAMBERTIAN_H_ */

   BBM_EXPORT_BSDFMODEL(bbm::lambertian);

The latter macro call (``BBM_EXPORT_BSDFMODEL``) informs BBM about the
existence of the bsdfmodel ``bbm::lambertian``.  This information can be used
to create any type of exporter (currently used for creating python bindings).

The former macro (``BBM_CHECK_CONCEPT``) is to aid the programmer in ensuring
that a struct/class meets a concepts during compilation.  In this case, it
check that the ''lambertian'' struct meets ''concepts::bsdfmodel''.

.. note::

   ``BBM_CHECK_CONCEPT`` activates the namespace ``bbm::concepts::archetype``
   which contains archetype definitions of many basic types such as
   ``config``, ``fresnel``, ``ndf``, etc...  It is important to note that
   these archetype types only meet the corresponding concepts; they do not
   offer an implementation.


Static bsdfmodel
================

A measured of static bsdfmodel follows exactly the same structure as a regular
``bsdfmodel`` albeit stored in ``include/staticmodel``.  A key difference is
that static bsdfmodels do not expose their attributes via reflection (i.e., no
``BBM_ATTRIBUTES``).  This means that ``BBM_DEFAULT_CONSTRUCTOR`` cannot be
used.  Instead the programmer must manually specify the constructor that takes
named arguments.

.. note::

   In order for ``BBM_EXPORT_BSDFMODEL`` to know how to construct the static
   bsdfmodel, a named constructor must be present.

BBM provides a helper macro ``BBM_CONSTRUCTOR`` for this:

.. doxygendefine:: BBM_CONSTRUCTOR
