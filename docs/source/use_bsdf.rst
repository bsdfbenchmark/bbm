BSDF Usage
==========

bbm::bsdfmodel
--------------

At the core of BBM's BSDF implementation lies the ``bsdfmodel`` concept.
Consider the following example (we assume ``BBM_IMPORT_CONFIG`` and ``use
namespace bbm`` has been executed in the current scope):

.. code-block:: c++

   Vec3d in(0,0,1);
   Vec3d out(0,0,1);
   lambertian<Config> model;

   std::cout << model << std::endl;
   std::cout << model.eval( in, out ) << std::endl;
   std::cout << model.sample( out, Vec2d(0.3, 0.7) ) << std::endl;
   std::cout << model.pdf( in, out ) << std::endl;
   std::cout << model.reflectance( out ) << std::endl;

This examples creates a ``lambertian`` BSDF model.  The output of the above
code is:

.. code-block:: none

   Lambertian(albedo = [0.5, 0.5, 0.5])
   [0.159155, 0.159155, 0.159155]
   ([-0.169256, 0.520915, 0.83666], 0.266317, Diffuse)
   0.31831
   [0.5, 0.5, 0.5]

By default ``albedo`` is set to ``Spectrum(0.5, 0.5, 0.5)`` as shown in the
first line.  Each BSDF model supports evaluation (``eval``), sampling
(``sample`` and ``pdf``), and querying an 'approximate' total reflectance.

.. note::

   BBM uses Eric Veach's convention for ``in`` and ``out`` directions, and
   transport flows from ``-in`` to ``out``. The exact direction (from light to
   camera or vice versa) depends on whether the unit of transport is
   ``radiance`` or ``importance``.  In case of ``radiance`` transport flows
   from light to camera.

Evaluating a BSDF models returns a ``Spectrum``. Note this does not include
foreshortening.

Sampling outputs a ``BsdfSample`` which contains the sampled direction
(``Vec3d``), the PDF of sampling this direction (``Value``), and the sampled
component.  The PDF can also be separately queried using the ``pdf`` method.
By default all components are sampled; one can restrict any of the four BSDF
model method to a single component:

.. code-block:: c++

   model.eval(in, out, bsdf_flag::Diffuse);    // eval only diffuse
   model.eval(in, out, bsdf_flag::Specular);   // eval only specular
   model.eval(in, out, bsdf_flag::Diffuse | bsdf_flag::Specular); // eval both diffuse and specular = bsdf_flag::All


Finally, the reflectance method produces an *approximate* estimate of the
directional total reflectance. While for the ``lambertian`` BSDF this
estimate is exact, for other models this is not necessarily the case.  In
general the total reflectance is used for determining which component to
sample, and and approximation is often sufficient.

Now consider, an example of the ``cooktorrance`` model:   

.. code-block:: c++

   cooktorrance<Config> ct1;
   cooktorrance<Config> ct2( Spectrum(0.1, 0.2, 0.3) );
   cooktorrance<Config> ct3( Spectrum(0.4), 0.3 );
   cooktorrance<Config> ct4( "eta"_arg = 1.1, "roughness"_arg = 0.3 );

   std::cout << ct1 << std::endl;
   std::cout << ct2 << std::endl;
   std::cout << ct3 << std::endl;
   std::cout << ct4 << std::endl;

This produces:

.. code-block:: none

   CookTorrance(albedo = [0.5, 0.5, 0.5], roughness = 0.100000, eta = 1.300000)
   CookTorrance(albedo = [0.1, 0.2, 0.3], roughness = 0.100000, eta = 1.300000)
   CookTorrance(albedo = [0.4, 0.4, 0.4], roughness = 0.300000, eta = 1.300000)
   CookTorrance(albedo = [0.5, 0.5, 0.5], roughness = 0.300000, eta = 1.100000)
                
The first model ``ct1`` is created with the default parameters.  We can pass
new values to each parameter following the same order as in the print out,
namely ``albedo``, ``roughness``, and ``eta`` (the Fresnel parameter); in the
2nd example we only change ``albedo`` and leave the remainder at their default
value, and in the 3rd example we change both ``albedo`` and ``roughness``.

The 4th example demonstrates a powerful flexible feature of BBM: named
arguments.  In this example, we only change ``roughness`` and ``eta``, and
leave ``albedo`` at its default value.  Note that the order does not matter
(i.e., we swapped the order of ``roughness`` and ``eta``), only the name
matters.  The name of the argument is specified by the literal ``""_arg``.

.. note::

   Providing an named argument that does not correspond to any of the
   arguments' names will result in a compile error indicating that no such
   method exists.  BBM matches named arguments at *compile* time.  This has
   the advantage that there is no run-time overhead.

Often BSDF models are combined, this can be achieved with ``aggregatemodel``:

.. code-block:: c++

   lambertian<Config> diff;
   cooktorrance<Config> spec;
   aggregatemodel< lambertian<Config>, cooktorrance<Config> model1(diff, spec);
   auto model2 = aggregate(diff, spec);

The ``aggregate`` method in conjunction with ``auto`` simplifies construction
of an ``aggregatemodel``.

.. note::

   A ``bsdfmodel`` does not contain ``virtual`` functions. It is intended to
   operate in conjunction with templates:

   .. code-block:: c++

      template<typename MODEL> requires concepts::bsdfmodel<MODEL>
        void foo(const MODEL& model)
      {
        ...
      }

The attributes of a ``bsdfmodel`` are publicly declared, and thus accessible
from outside the class.  However, bsdf attributes also contain additional
information/properties:

.. code-block:: c++

   lambertian<Config> model
   std::cout << "value: " << model.albedo << std::endl;
   std::cout << "default value: " << default_value(model.albedo) << std::endl;
   std::cout << "lower bound: " << lower_bound(model.albedo) << std::endl;
   std::cout << "upper bound: " << upper_bound(model.albedo) << std::endl;
   std::cout << "attribute flag: " << bsdf_attr_flag(model.albedo) << std::endl;

This will produce:

.. code-block:: none

   value: [0.5, 0.5, 0.5]
   default value: [0.5, 0.5, 0.5]
   lower bound: [0, 0, 0]
   upper bound: [1, 1, 1]
   attribute flag: Diffuse Scale

With exception of the value of the attribute, this additional information is
constant and does not increase the storage requirements of the attribute.
These properties are not enforced (i.e., one can assign a value larger than
the upper bound), but will serve to guide other methods on how to use the
attributes (e.g., bsdfmodel construction or parameter fitting).


bbm::bsdf
---------

Templated bsdfmodels are well suited if the bsdf model is known beforehand,
e.g., if the application only uses a single model.  However, certain
applications require a polymorphic bsdf.  BBM provides a wrapper class called
``bsdf`` that provides an interface for bsdfmodels:

.. code-block:: c++

   bsdf<lambertian<Config>> b;
   std::cout << b << std::endl;
   std::cout << b.eval( in, out ) << std::endl;

   const bsdf_base<Config>& base = b;
   std::cerr << base << std::endl;
   std::cout << base.eval( in, out ) << std::endl;
   
The output of the above code is exactly the same as calling the same methods
on the bsdfmodel ``lambertian<Config>``, except that the calls are now
virtual.  The abstract interface of ``bsdf<...>`` is ``bsdf_base``:
   
.. doxygenstruct:: bbm::bsdf_base
   :members:

A ``bsdf`` is also a ``bsdfmodel`` and thus meets ``concepts::bsdfmodel``.  A
``bsdf`` also must meet ``concepts::bsdf``:

.. doxygenconcept:: bbm::concepts::bsdf


Similar to ``aggregatemodel``, several ``bsdf`` objects can be aggregated in a
``aggregatebsdf``:

.. code-block:: c++

   aggregatebsdf<Config> model( bsdf<lambertian<Config>>(), bsdf<cooktorrance<Config>>() );

   auto model2 = aggregate( bsdf<lambertian<Config>>(),
   bsdf<cooktorrance<Config>>() );

The ``aggregate`` method will decide based on its arguments whether to create
an ``aggregatemodel`` or an ``aggregatebsdf``.  Internally, a
``aggregatebsdf`` stores ``bsdf_ptr`` objects.  This is essentially a ``bsdf``
that owns a smart-pointer to a ``bsdf`` object.

.. doxygenstruct:: bbm::bsdf_ptr
   :members:
   :private-members:
             

Python Libraries
----------------

If compiled with ``BBM_PYTHON_LIBRARIES=ON``, then a python module will be
generated for each configuration.  Assume ``BBM_NAME='bbm'``, then:

.. code-block:: python

   import bbm_floatRGB as bbm
   model = bbm.Lambtian([0.1, 0.2, 0.3])
   str( model.eval([0,0,1], [0,0,1]) )

   param = bbm.parameter_values(model)
   param[0] = 1
   str(model)

All bsdfmodels are named similarly (except for capitalization), and the four
core methods operate similarly as in ``C++``. The output of the above would
be:

.. code-block:: none

   '(0.03183099, 0.06366198, 0.09549297)'
   'Lambertian(albedo = [1, 0.2, 0.3])'

   
.. note::

   Currently BBM only allows indirect access to the brdf parameters via the
   ``parameter_values`` method which linearizes the attributes into a vector.
   Direct access to the BSDF attributes is part of the 'wish list' for future
   versions of BBM.

   
