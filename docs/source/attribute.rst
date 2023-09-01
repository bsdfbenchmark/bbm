bbm::attribute
==============

BBM attributes (not to be confused with class attributes) are a wrapper for
any type that allows the programmer to attach additional compile time
information.  BBM uses attributes in two ways:

1. to attach a tag to a type to differentiate between different
   interpretations of the value (cf. a unit like 'ft' or 'cm').  BBM uses this
   to differentiate between index of refraction and reflectance at normal
   incidence. Both are used to parameterize the Fresnel reflectance equations.

2. to attach default value, bounds, and type to an BSDF parameter, implemented
   through ``bbm::bsdf_parameter``.

``bbm::attribute`` is implemented in `core/attribute.h
<../doxygen/html/core_2attribute_8h_source.html>`_ and meets
``bbm::concepts::attribute``:

.. doxygenconcept:: bbm::concepts::attribute


There are two specializations of ``bbm::attribute``: one for scalar types and
one for non-scalar types.  Both operate similarly with the key difference that
``bbm::atrribute`` stores a scalar type as a class member and it inherits from
the non-scalar type (in order to inherit its functions).

``bbm::attribute`` takes one template parameter that must meet
``bbm::concepts::attribute_property`` which describes the type and any other
optional information one wants to attach to the type:

.. doxygenconcept:: bbm::concepts::attribute_property

The ``bbm::attribute`` implementation adds a few extra features besides the
requirements set in ``bbm::concepts::attribute``:

1. If constructed from a type that meets ``concepts::constructible``, then a
   custom ``convert`` method is called to convert the value of the
   construction type to the attribute type.   This is for example used to
   correctly covert from index of refraction to reflectance and vice versa.

2. Math operations are defined to automatically forward to the underlying
   type.

3. ``bbm::attribute`` forwards any valid cast operation to the underlying
   type.

``bbm::attribute`` is defined to be transparent by automatically casting to
the underlying type when required.  However, this does not (annoyingly)
always work. For example in the following case:

.. code-block:: c++

   template<typename T> requires std::is_scalar_v<T>
     void foo(const T& t) {}

   void bar(int t) {}
     
   struct prop { using type = int; };

   bbm::attribute<prop> scalar;
   bar(scalar);                   // OK
   foo(scalar);                   // compile error

In the case of ''bar'' the attribute scalar is automatically cast to an
``int``.  However, in the case of ``foo``, the compiler does not know which
type what type to cast ``scalar`` to; ``C++`` lacks the ability to examine
the intersection of types that meet the constraints and types ``scalar`` can
be cast to.

bbm::value
----------

In certain cases automatic casting of attributes to their underlying type does
not work in the following case:

.. code-block:: c++

   template<typename T> requires std::is_integral<T>
     void foo(const T& t);

Even if the underlying type is integral, C++ does not know that it should
first cast the attribute to its type.  To alleviate this, BBM includes
(defined in `util/attribute_value.h
<../doxygen/html/attribute__value_8h_source.html>`_:

.. doxygenfunction:: bbm::value

This method will cast the attribute to its value type (or leave it unaltered
if not an attribute).  Similarly, sometimes the type of the underlying type is
needed, but it is not always the case that the type is an attribute:

.. doxygentypedef:: bbm::attribute_value_t

will either return the type of ``T`` or, if T is an attribute, ``typename T::type``.
