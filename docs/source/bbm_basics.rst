BBM Basics
==========

Lets take the previous example again:

.. code-block:: c++

   #include "bbm.h"
   using namespace bbm;

   int main(int argc, char** argv)
   {
     BBM_IMPORT_CONFIG( floatRGB );

     return 0;
   }

The header file ``bbm.h`` includes all relevant header files for BBM and sets
up the BBM backbone.  All BBM code resides in the ``bbm`` namespace, hence we
import the namespace bbm.  Some methods might clash with methods with the same
name in other namespaces, and might still require explicitely adding
``bbm::``.

bbm::config
-----------

The macro ``BBM_IMPORT_CONFIG( <config name> )`` sets up some convenient type
aliases based on the chosen bbm configuration. The number of available
configurations depend on the backbone.  For example, the ``native`` backbone
currently only supports:

* ``floatRGB``: scalar non-differentiable/non-packet floating point types.
* ``doubleRGB``: scalar non-differentiable/non-packet double precision
  floating point types.

The goal of a configuration is to easily template classes (e.g., BSDFs) with
common basic types such as ``Value`` and ``Spectrum``.  Other
backbones support additional configurations (e.g., ``floatDiffRGB`` and
``floatPacketRGB``), and future configurations are possible. It is also
possible to create your own configuration. BBM relies on ``C++ concepts`` for
setting specification requirements.  A config is a structure that must meet
the following requirements:

.. doxygenconcept:: bbm::concepts::config_struct

The macro ``BBM_IMPORT_CONFIG( <config name> )`` will take the information
from the config, and makes the following aliases active in the current scope:

* ``Config``: an alias to the imported config. Use this alias to pass to BBM
  classes instead of the actual name of the BBM config. This way, one only
  need to change the ``BBM_IMPORT_CONFIG`` call to change the configuration.
* ``Value``: same as ``Config::Value``
* ``Spectrum``: same as ``Config::Spectrum``
* ``Scalar``: underlying scalar type (typically ``float`` or ``double``).
  This can be different from ``Value`` (e.g., in the case when value is a
  packet or differentiable type).
* ``Mask``: in its simplest form this is equivalent to ``bool``. However,
  some configurations might require more complex types that somewhat mimics
  the behavior of a ``bool``.  However, similar to ``DrJIT`` and ``Enoki``,
  when the underlying ``Value`` is a packet type, then ``Mask`` is equivalent
  to an array of ``bool``.  Hence, we cannot directly compare a
  ``Mask``. Instead use methods such as ``bbm::none(Mask)``,
  ``bbm::any(Mask)``, ``bbm::all(Mask)`` to reduce the ``Mask`` to a ``bool``,
  and ``bbm::select(Mask, A, B)`` to replace ``(Mask ? A : B)``.
* ``Size_t``: integer version of ``Value``.  If ``Value`` is a packet type,
  then ``Size_t`` will also be a packet.
* ``BsdfFlag``: Will be a packet if ``Value`` is a packet too.

  .. doxygenenum:: bsdf_flag
  
* ``Constants``: type dependent constants:

  .. doxygenstruct:: bbm::constants
     :members:
     :undoc-members:
  
* ``Vec2d`` and ``Vec3d``: 2D and 3D vectors in which each element is of type
  ``Value``.
* ``Mat2d`` and ``Math3d``: 2x2 and 3x3 matrices in which each element is of
  type ``Value``
* ``Complex``: a complex type in which each component is of type ``Value``.
* ``BsdfSample``: structure to hold a sampled direction and pdf:

  .. doxygenstruct:: bbm::bsdfsample
     :members:
                     
* ``Vec3dPair``: a pair of in and out directions:

  .. doxygenstruct:: bbm::vec3dpair
     :members:
                     
* ``BsdfPtr``: pointer type to a bsdf.


.. note::

   ``BBM_IMPORT_CONFIG`` is recursive. Any type that itself has a bbm-config
   imported in its scope, can also serve a config.

   .. code-block:: c++

      struct Foo { BBM_IMPORT_CONFIG( floatRGB ); };
      struct Bar { BBM_IMPORT_CONFIG( Foo ); };       // will use Foo::Config = floatRGB
  
An example of using ``BBM_IMPORT_CONFIG``:

.. code-block:: c++

   #include <iostream>
   #include "util/typestring.h"
   
   #include "bbm.h"
   using namespace bbm;

   template<typename CONF> requires concepts::config<CONF>
   struct Foo
   {
      BBM_IMPORT_CONFIG( CONF );
      Value val;
   };
   
   int main(int argc, char** argv)
   {
     BBM_IMPORT_CONFIG( floatRGB );

     Vec2d v(1, 2);
     Foo<Config> foo;
     
     std::cout << v << std::endl;
     std::cout << toTypestring(v) << std::endl;
     std::cout << bbm::typestring<Specrtum> << std::endl;
   }

The above example, defines a struct ``Foo`` that takes a config as template
parameter ``CONF``.  Here we use ``concepts::config<CONF>`` to check that
``CONF`` is indeed a valid config.

After ``BBM_IMPORT_CONFIG`` we can use any of the aliases.  For example, in
the main body we declare a variable ``Vec2d v``, and a instance of
``Foo``. Note that in the latter case we use the ``Config`` alias; if at any
point we decide to change the configuration, we only need to change the
``BBM_IMPORT_CONFIG`` line.

.. note::

   All concepts in bbm are organized in the ``bbm::concepts`` namespace.


.. note::

   ``utils/typestring.h`` contains two useful tools for debugging.
   ``toString`` is a macro that takes first extract the type of the argument,
   and then calls the const-expression ``bbm::typestring<...>`` to convert the
   type to a human-readable ``std::string_view``.


Basis operations
----------------

All basis types support a number of operations implemented in the backbone:
`Math operations`_, `Horizontal operations`_, `Comparison operations`_, and `Control operations`_.
In addition, each type as a corresponding mask type that can be obtained with
the ``bbm::mask_t<...>`` expression with corresponding `Mask operations`_.


Math operations
~~~~~~~~~~~~~~~

.. doxygenconcept:: bbm::concepts::backbone::has_math_functions

Horizontal operations
~~~~~~~~~~~~~~~~~~~~~

.. doxygenconcept:: bbm::concepts::backbone::horizontal

Comparison operations
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenconcept:: bbm::concepts::backbone::ordered

Control operations
~~~~~~~~~~~~~~~~~~

.. doxygenconcept:: bbm::concepts::backbone::control
                    

Mask operations
~~~~~~~~~~~~~~~

.. doxygenconcept:: bbm::concepts::backbone::horizontal_mask


Non-scalar Operations
---------------------

``Vec2d``, ``Vec3d``, and ``Spectrum`` are multi-value types that support
element-wise lookup with ``operator[]``.

Additional Vector Operations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Vectors have additional helper functions defined in the ``bbm::vec``
namespace:

.. doxygennamespace:: bbm::vec
   :members:

In addition the following methods are available in the ``bbm`` namespace:

.. doxygenfunction:: bbm::perp

.. doxygenfunction:: bbm::cperp

.. doxygenfunction:: bbm::reflect(const vec3d<T>& v)

.. doxygenfunction:: bbm::reflect(const vec3d<T>& v, const vec3d<T>& normal)

.. doxygenfunction:: bbm::cross

.. doxygenfunction:: bbm::halfway

.. doxygenfunction:: bbm::convertToHalfwayDifference

.. doxygenfunction:: bbm::difference

.. doxygenfunction:: bbm::convertFromHalfwayDifference                     

Spherical Coordinates
~~~~~~~~~~~~~~~~~~~~~

The following additional vector operations are included to support spherical
coordinates:

.. doxygennamespace:: bbm::spherical
   :members:

                     
Matrix
------

Matrices are implemented based on the vector implementations provided by the
backbone.  It support element lookup with ``operator(row, col)`` and supports
additional ``transpose`` and ``identity`` methods.

Matrix Transformations
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: bbm::rotationX(T value)

.. doxygenfunction:: bbm::rotationX(const vec2d<T>& cossin)

Similar functions exists for the Y and Z axes.
                     

Shading Frame
~~~~~~~~~~~~~

.. doxygenfunction:: bbm::toGlobalShadingFrame

.. doxygenfunction:: bbm::toLocalShadingFrame                     


