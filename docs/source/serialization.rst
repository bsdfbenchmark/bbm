BSDF Serialization
==================

bbm::toString
-------------

BBM offers convenient serialization of bsdfmodels (and bsdfs) to string:

.. code-block:: c++

   lambertian<Config> model;
   std::string model_string = bbm::toString(model);
   std::cout << model_string<< std::endl;

The serialization notation is the same as the python notation (for the regular
``bbm_floatRGB`` configuration).

``bbm::toString`` is defined for many types, including the STL ``std::array``,
``std::vector``, and ``std::tuple``, as well as BBM types such as attributes,
classes that support reflection, and named tuples.

.. note::

   ``bbm::toString`` is the correct way to serialize backbone core types such
   as ``Vec2d``, ``Vec3d``, ``Spectrum``, and ``Complex``.  It offers a
   consistent serialization regardless of how the underlying library might
   stream the types to cout.  For example, BBM serializes a ``Complex`` type
   as ``[R I]``, whereas enoki/drjit will stream a complex type as
   ``R + iI``.
   

bbm::fromString
---------------

BBM also supports deserialization using the ``bbm::fromString`` method:

.. code-block:: c++

   std::string str = "Lambertian([1,2,3])";
   auto model = bbm::fromString< lambertian<Config> >(str);
   std::cout << model << std::endl;

Note that you need to specify by explicitely passing as a template parameter
what the target type is (``lambertian<Config>`` in this case).
Deserialization works for any type that can be sucessfully serialized.

.. note::

   The only exception to the serialization/deserialization symmetry are
   types that support attribute reflection but that are not trivially
   constructable (i.e., without any parameters).

bbm::fromString<bsdf_ptr<Config>>
---------------------------------

A special case of deserialization is to the ``bsdf_ptr`` type.  In this case,
the deserialization will determine based on the string what the underlying
bsdfmodel is.

.. note::

   Deserialization to a ``bsdf_ptr`` will only consider the bsdfmodels
   included in ``BBM_BSDFMODELS`` (it will always include aggreagte models).

.. warning::

   Including all bsdmodels in ``BBM_BSDFMODELS`` will greatly increase
   compilation time when using ``bbm::fromString<bsdf_ptr<Config>>`` because
   each bsdfmodel needs to be delared and fully compiled.
