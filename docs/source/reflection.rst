Reflection
==========

Problem Statement
-----------------

Lets consider the following didactic example:

.. code-block:: c++

   struct value
   {
     float a;
   };
   
   struct value_pair
   {
      float a, b;
   };

   float squared_sum(const value& v) { return v.a*v.a; }
   float squared_sum(const value_pair& v) { return v.a*v.a + v.b*v.b; }

Each time we add a new struct for which we want to be able to compute the
``squared_sum`` we need to define an additional specialization.  In this
particular case the specialization is not particularly complex, but it does
create overhead for anyone implementing new types.  Furthermore, if at some
point we need an ``abs_sum`` then we have to implement this for all types.

Reflection offers an alternative approach by exposing the attributes
anonymously. The above example would become:

.. code-block:: c++

   struct value
   {
     float a;
     BBM_ATTRIBUTES(a);
   };

   struct value_pair
   {
     float a, b;
     BBM_ATTRIBUTES(a, b);
   };

   template<typename T> requires bbm::concepts::reflection::attributes<T>
     float squared_sum(const T& v)
   {
     float sum = 0;
     CONSTFOR(idx, bbm::reflection::aatributes_size<T>,
     {
        sum += std::pow( std::get<idx>( bbm::reflection::attributes(v) ), 2.0);
     });
     return sum;
   }

While more to complex to write, it is more generic; the method will now work
for any struct which reflects its attributes.

BBM implements a light-weight compile-time reflection mechanism that supports
reflection of attributes and base classes.  Currently not supported are member
functions.  Before detailing how reflection is implemented, we first explore
how to use it.

Usage
-----

Attributes
~~~~~~~~~~

As shown in the above didactic example, to reflect the attributes of a struct
or class, we invoke a macro ``BBM_ATTRIBUTES(...)`` with a comma separated
list of attributes to reflect.  This macro can only be invoked after the
attributes have been declared.  For example:

.. code-block:: c++

   struct foo
   {
     float bar;
     char var;

     BBM_ATTRIBUTES(bar, var);
   };

We can then query the attributes, types, and number of attributes:

.. code-block:: c++

   using namespace bbm::reflection;
   foo f{1, 'a'};
   
   std::cout << attributes(f) << std::endl;  // (bar = 1, var = 'a')
   std::cout << bbm::typestring< attributes_t<foo> > << std::endl;  // named< std::tuple<float&, char&>, "bar", "var">
   std::cout << attributes_size<foo> << std::endl;  // 2
   std::cout << bbm::typestring< std::tuple_element_t<0, attributes_t<foo>> > << std::endl; // float

.. note::

   All reflection methods (e.g., ``attributes(...)``) and type traits (e.g.,
   ``attributes_t<...>``) are defined in the ``bbm::reflection`` namespace.

The above example shows that the return type attributes can be queried with
``attributes_t``.  However, this only works in an evaluated context (i.e., the
type of the class must be fully known). A macro provides a similar
functionality that also works in unevaluated contexts:

.. code-block:: c++

   struct foo1
   {
      float a, b;
      BBM_ATTRIBUTES(a, b);

      using A_t = BBM_ATTRIBUTES_T;  // using attributes_t<foo1> here throws an error.
   };
   
BBM reflection also allows for includes the attributes of an attribute:

.. code-block:: c++

   struct foo2
   {
     float a;
     foo b;
     BBM_ATTRIBUTES(a, attributes(b));
   };

   foo2 f2;
   std::cout << attributes(f2) << std::endl;  // (a = 0, bar = 0, var = 0)

Note that the attributes are `flattened` into a flat structure.  If one
desires a non-flattened structure, then ``BBM_ATTRIBUTES(a, b)`` achieves
this.

The above examples reveal that ``attributes(...)`` returns a named tuple,
which the type is a reference to the attribute type and the name corresponds to
the attribute name.  This means that all methods available on named types can
also be applied (e.g., ``bbm::get<"a">(f2)``).

Base class
~~~~~~~~~~

To enable reflection of base classes, we include an additional macro
``BBM_BASETYPE``:

.. code-block:: c++

   struct foo3 : public foo
   {
     BBM_BASETYPES(foo);

     float bar;
     BBM_ATTRIBUTES(bar);
   };

Multiple base classes can be passed to ``BBM_BASETYPES`` as a comma separated
list.  We can query the base class types as:

.. code-block:: c++

   std::cout << bbm::typestring< basetypes_t<foo3> > << std::endl;  // std::tuple<foo>
   std::cout << bbm::typestring< std::tuple_element_t<0, basetypes_t<foo3> > << std::endl; // foo
   std::cout << basetype_size<foo3> << std::endl; // 1

.. note::

   BBM reflection currently does not store a `string_literal`` of the base
   class names. This is difficult due to template parameters (and thus
   potentially commas) in base class names.  If the names of base classes is
   required, then a solution is to store a ``static constexpr string_literal
   name`` in each class (i.e., ``bbm::concepts::named``).

When a base type is specified, BBM will automatically include the base class
reflected attributes when querying for the attributes of the specialized
class:

.. code-block:: c++

   foo3 f3;
   std::cout << attributes(f3) << std::endl; // (bar = 0, var = 0, bar = 0)

However, this might cause a clash in attribute names (as with bar in this
case).  This is not an issue as long as you do not want to query an attribute
by its name (using ``bbm::get<"name">``).  If unique names are required, then
'prefixing' the attribute names of a base class is a solution:

.. code-block:: c++

   struct foo4 : public foo, foo3
   {
     BBM_BASETYPES(prefix<"foo::", foo>, foo2);
   };

   foo4 f4;
   std::cout << attributes(f4) << std::endl; // (foo::bar = 0, foo::var = 0, bar = 0)

.. note::

   Both ``attributes`` and ``basetype_t`` operate correctly when applied to a
   struct/class for which no reflection has been defined.  Care must be taken
   when defining a class inside another class. If no reflection is specified,
   the reflection of the outer class is copied, which might not produce the
   desired result.

Implementation Details
----------------------

When calling the ``BBM_ATTRIBUTES`` macro the a typedef ``attribute_tuple_t``
and a method ``attribute_tuple(void)`` is added to your class.  Without base
class reflection this method returns:

.. code-block:: c++

   auto attr =  bbm::named_flatten( bbm::make_named<BBM_STRINGIFY_EACH(__VA_ARGS__)>(bbm::make_ref_tuple(__VA_ARGS__)) );

Lets decipher this:

1. ``__VA_ARGS__`` contains the comma separated list of attributes.

2. By calling ``bbm::make_ref_tuple(__VA_ARGS__)`` we create a tuple where
   each type is a reference to the corresponding class attribute.

3. Next, we convert this tuple into a named tuple using ``bbm::make_named``.
   However, we need to pass a comma separated list of attribute names. The
   macro ``BBM_STRINGIFY_EACH(__VA_ARGS__)`` does this. For example suppose
   ``__VA_ARGS__ = a, b, c``, then the result of this macro class is ``"a",
   "b", "c"``. Please refer to `include/util/macro_util.h
   <../doxygen/html/macro__util_8h_source.html>`_ for details on the macro
   implementation.

4. Finally, to support recursive attributes, we flatten the named tuple.

The typedef ``attribute_tuple_t`` essentially does the same inside a
``decltype``.

When defining base type reflection with ``BBM_BASETYPES`` a typedef
``reflection_base_t`` is added to the class.  It will alias a class
``bbm::reflection::detail::base_types`` that defines:

1. a typedef to ``type``: a tuple of base class types.
2. a typedef to ``attribute_t``: a named type of the attribute tuples of 
   all base classes (concat in a single named type).
3. a method ``attribute_tuple(*this)`` that return the values of the attributes from
   all base classes (concat in a single named type).

The method ``attribute_tuple`` defined by ``BBM_ATTRIBUTES`` then performs an
additional ``bbm::named_cat``:

.. code-block:: c++

   inline constexpr auto attribute_tuple(void)
   { 
     auto attr = bbm::named_flatten(bbm::make_named<BBM_STRINGIFY_EACH(__VA_ARGS__)>(bbm::make_ref_tuple(__VA_ARGS__)));
     return bbm::named_cat(attr, reflection_base_t::attribute_tuple(*this));
   }

.. note::

   In order to make reflection robust to cases where no attributes reflected,
   a global alias ``attribute_tuple_t = reflection_base_t::attribute_type_t``
   (equivalent to ``bbm::named<std::tuple<>>``) is defined.  Similarly, a
   global alias ``reflection_base_t`` is defined.  Consequently,
   ``attribute_tuple_t`` and ``reflection_base_t`` should be considered
   reserved.

   
Usage in BBM
------------

Reflection is used in three ways in BBM.

1. the methods ``parameter_values``, ``parameter_default_values``,
   ``parameter_lower_bound``, and ``parameter_upper_bound`` relies on
   reflection to enumerate all attributes, possibly flattening iterable
   attribute types (e.g., ``Spectrum``).  Similarly, reflection is also used
   to stream the attributes of ``bsdfmodel`` to an ostream.

2. all bbm backbone math expressions have been extended to also operate on
   structs that support attribute reflection.  In that case, the operation is
   perform of each reflected attribute.  This means that any class/struct that
   reflects its attributes will automatically be supported by the bbm backbone
   math expressions.  Similarly, ``bbm::select`` also leverages reflection to
   extend its functionality to such structs.

3. ``BBM_DEFAULT_CONSTRUCTOR`` leverages reflection to create the argument
   list for the constructor, and to copy the arguments to each of the
   reflected attributes.
