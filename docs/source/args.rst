Named Arguments
===============

In order to support automated exporting to Python, BMM needed a way to encode
the function argument list of a constructor.  This idea further expanded to
also relax the rigid way arguments are passed in ``C++``.

Named arguments consist of two parts, the specification of individual function
arguments (`bbm::arg`_), and a structure for specifying a sequence of
arguments (`bbm::args`_).

bbm::arg
--------

``bbm::arg`` is a structure to store the relevant information (compile and
run-time) that mimics the behavior of an argument. This information is:

1. argument type
2. argument name (stored as a ``bbm::string_literal``)
3. the default value (passed as a stateless lambda function)
4. the value of the argument; including support for rvalues.

This is achieved via template specialization.  The definition of ``bbm::arg``:

.. doxygenstruct:: bbm::arg

A typedef ``bbm::is_arg_v`` is included to detect if a type is a ``bbm::arg``.

The five specializations are:

1. an untyped arguments: ``bbm::arg<void, Name, void>``.  One can assign a
   value to this argument.  The resulting type of the assignment, however, is
   one of the four other specializations. I.e., the result of the assignment
   is a different type.  The custom string literal ``""_arg`` creates this
   specialization:

   .. code-block:: c++

      auto a = "test"_arg;
      std::cout << toTypestring(a) << std::endl; // bbm::arg<void, bbm::string_literal<5>{"test"}, void>
      std::cout << a << std::endl; // test

      auto b = (a = 3); // changes type on assignment
      std::cout << toTypestring(b) << std::endl; // bbm::arg<const int&, bbm::string_literal<5>{"test"}, void>
      std::cout << b << std::endl; // const int& test = 3

   A ``bbm::arg`` contains a typedef alias to its ``type``, and a ``static
   constexpr string_literal name;``
      
2. a non-reference type without default value: ``bbm::arg<Type, Name, void>``
   with requirement: ``(!std::is_void_v<Type> &&
   !std::is_reference_v<Type>))``.  This corresponds to the type of ``b`` in
   the previous example.  This ``bbm::arg`` variant has a private class
   attribute: ``_value`` to store the argument value. In addition to the
   assignment operator and constructors, this specialization also contains
   cast operators to the underlying value type.  Note: the assignment operator
   will assign the value to ``bbm::arg`` and ``*this`` is returned.

3. a non-reference type with default value: ``bbm::arg<Type, Name, Default>``
   with the requirement that: ``(!std::is_void_v<Type> &&
   !std::is_reference_v<Type>) && std::invocable<Default>``.  The latter
   constraint implies that ``Default`` is a lambda function or a functor.  The
   difference with ``bbm::arg`` without default value is that the trivial
   constructor sets the value to the default value.

   To facilitate passing a default value, a helper macro ``ArgDef(...)`` is
   defined that creates a stateless lambda that return the macro argument.

   .. code-block:: c++

      bbm::arg<float, "test", ArgDef(4.0)> a;
      std::cerr << a << std::endl;  // float test = 4.000000 [ = 4.000000]

      a = 3.0;
      std::cerr << a << std::endl; // float test = 3.000000 [ = 4.0000000]
      
4. a reference type without default value: ``bbm::arg<Type, Name, void>`` with
   the requirement: ``(!std::is_void_v<Type> && std::is_reference_v<Type>)``.
   The private class attribute is declared as:
   ``bbm::persistent_reference<Type> _value``.  A key difference with the
   previous two specialization is that the assignment operator *re-assigns*
   the reference, not the value!  A custom cast operator to ``const type&``
   and ``type&`` is included.

   .. code-block:: c++

      float f = 3.0;
      auto a = ("test"_arg = f);
      std::cout << toTypestring(a) << std::endl; // bbm::arg<float&, bbm::string_literal<5>{"test"}, void>
      std::cout << a << std::endl; // float& test = 3.000000

      f = 4.0;
      std::cout << a << std::endl; // float& test = 4.000000

   The first half, we can see that assigning a variable 'f' creates as
   expected a ``bbm::arg`` with a reference to ``f``.  In the second half we
   can see that changing ``f`` is reflected in ``a`` (because it is a
   reference to ``f``).
      
5. a reference type with default value: exactly the same as the previous with
   exception of the trivial constructor.  The underlying
   ``bbm::persistent_reference`` takes care of keeping rvalues in scope.

   .. code-block:: c++

      bbm::arg<const float&, "test", ArgDef(4.0)> a;
      std::cout << a << std::endl;  // const float& test = 4.000000 [ = 4.000000] 

      bbm::arg<const float&, "test2", ArgDef(4.0)> b = 3.0;
      std::cout << b << std::endl;  // const float& test2 = 3.000000 [ = 4.000000] 

      float f = 2.0;
      bbm::arg<const float&, "test3", ArgDef(4.0)> c = f;
      std::cout << c << std::endl;  // const float& test3 = 2.000000 [ = 4.000000] 

   ``test`` will reference a temporary float that contains the value ``4.0``
   (set from the default value). ``test2`` also creates a temporary float in
   which the value ``3.0`` is copied.  ``test3`` contains a reference to ``f``
   without allocating any new temporary memory.

bbm::args
---------

``bbm::args`` is a collection of ``bbm:arg``:

.. doxygenstruct:: bbm::args
   :outline:

Usage Examples
~~~~~~~~~~~~~~

.. code-block:: c++

   using namespace bbm;
   void foo( args<arg<float, "a">, arg<float, "b">> myargs ) {}

   // either pass with {} or by explicit cast
   foo( {1, 2} ); 
   foo( args<arg<float, "a">, arg<float, "b">>(1, 2) );

   // can use names or position to pass
   foo( {"a"_arg = 1, "b"_arg = 2} );
   foo( {"b"_arg = 2, "a"_arg = 1} );
   foo( { 1, "b"_arg = 2} );

   // pass by explicit denote position:
   foo( {"0"_arg = 1, "1"_arg = 2} );
   foo( {"1"_arg = 1, "0"_arg = 2} );

The above example shows different ways to pass arguments.

1. One can embed the arguments in curly brackets (i.e., an initializer list).
   If the compiler is not able to figure out the type, explicit casting is
   required (2nd example).

2. We can also exploit the custom literal ``""_arg`` to pass arguments.  Note
   that the constructor of ``bbm::args`` will match ``bbm::arg`` by name when
   constructing ``bbm::args``.  If no name is provided, ``bbm::args`` use the
   position in the list to assign it to the correct ``bbm::arg``.

3. The ``bbm::args`` constructor will interpret numerical argument names as
   positions in the argument list. Hence, ``"0"_arg = 1`` assign ''1'' to the
   zero-th ``bbm::arg``. 

4. Another assignment mechanism (not shown) is when the type of the argument
   is unique. In that case, all of the above solution for matching fail, the
   argument will be assigned based on compatible (unique) type.

To access the values stored in a ``bbm::args``:

.. code-block:: c++

   std::cout << myargs.template value<0>() << std::endl;  // zero-th argument
   std::cout << myargs.template value<"a">() << std::endl; // argument named "a"
   std::cout << myargs.value("a"_arg) << std::endl; // argument named "a"

Alternatively, a helper macro can create aliases:

.. code-block:: c++

   BBM_IMPORT_ARGS(myargs, a, b);
   std::cerr << a << std::endl;    // alias 'a' is create to args.value("a"_arg)

``bbm::args`` contains a number of other useful typedefs and methods:

.. code-block:: c++

   using T = decltype(myargs);
   
   myargs.size;  // number of arguments
   T::size;      // number of arguments
   
   myargs.values() // type with all (references to) the argument values

   myargs.get("a"_arg);  // return bbm:arg matching "a"
   myarg.value("a"_arg); // return bbm::arg::type matching "a"

   T::is_compatible<int, std::string>; // true if passing (int, std::string) can be used to construct T
   T::is_cpp_compatible<int, float>;   // true if passing (int, float) is compatible when using classic C++ passing mechanism.

Having the encapsulate the arguments in curly brackets or by explicit casting
is a but cumbersome.  Therefore, BBM has four different macros to help create
a specialized function that takes a parameter pack as input, and forwards it
to a method with a ``bbm::args`` argument.  The ``is_compatible`` constexpr
boolean is used to constrain the forwarding method to only compatible packs:

.. code-block:: c++

   BBM_FORWARD_ARGS(foo, arg<float, "a">, arg<float, "b">);

   foo(1, 2);               // forwarded
   foo(1, "b"_arg = 2);     // forwarded
   foo("a"_arg = 1, "b"_arg = 2); // forwarded

``BBM_FORWARD_ARGS_CONST`` defined the forwarding method as a ``const``
method.

The above macro still expects that the method that is being forwarded to has a
``bbm::args`` argument.  However, it is also possible to pass to a regular
function (i.e., basically extend the flexibility in how the method can be
called):

.. code-block:: c++

   void bar( float a, int b=1 ) {}

   BBM_FORWARD_CPP_ARGS(bar, arg<float, "a">, arg<int, "b", ArgDef(1)>);

   bar(1);    // direct cpp call; no forwarding
   bar(1, 2); // direct cpp call; no forwarding
   bar("b"_arg = 2, "a"_arg = 1); // forwarding

Similar as before ``BBM_FORWARD_CPP_ARGS_CONST`` defines the forwarding method
as ``const``.

.. warning::

   The ``bbm:arg`` list passed to ``BBM_FORWARD_CPP_ARGS`` must match exactly
   the definition of the target method, *including* default arguments.  This
   is necessary to determine whether or not the method can be called directly
   (``is_cpp_compatible``).

   
Implementation
~~~~~~~~~~~~~~

Please refer to `include/core/arg.h <../doxygen/html/arg_8h_source.html>`_
and `include/core/args.h <../doxygen/html/args_8h_source.html>`_ for
the full implementation details.  In what follows, we will focus on how BBM
resolves which constructor argument to assign to which ``bbm::arg``.

The whole process of matching arguments is performed in ``constexpr`` during
compile time.  Hence, there is no run-time overhead.  When passing a series of
arguments to the constructor of ``bbm::args``, the constructor forwards them
as a tuple to a private method ``_retrieve_args`` which creates each
``bbm::arg`` in order as defined in ``bbm::args``(using the private method
``_retrieve_arg``) by searching through the forwarded tuple of constructor
arguments for a matching constructor argument.  Hence, the (compile) time
complexity is roughly squared with respect to the number of arguments.  If no
such element is found in the forwarded constructor arguments, then the default
value of the target ``bbm::arg`` is tried.  If no default argument exists, a
compile error is generated.

The key method in finding a matching argument in the forwarded constructor
arguments is the ``_find_arg_index`` method that takes the index of the
``bbm::arg`` we are trying to match, and the forwarded tuple type (we do not
need to values to determine which argument is the best match), and it returns
the index (in the forwarded constructor arguments) that is the best match.

``_find_arg_index<IDX, TUP>`` proceeds in the following order:

1. we scan the whole TUP to see if there is an element with a matching name to
   the IDX-th ``bbm::arg``. If a match is found, *and* the type is compatible with
   the IDX-th ``bbm::arg``, then the search process is terminated the matching
   index (in TUP) is returned.

2. if there exists an argument in TUP with name "IDX" and the type is
   compatible, then the matching index (in TUP) is returned.

3. check if TUP[IDX] is compatible with the IDX-th ``bbm::arg``. If the
   TUP[IDX] element is also a ``bbm::arg``, then the name must be empty (to
   avoid assign ``"b"_arg = 1`` at position ``0`` to a `bbm::arg`` with a
   different name). If compatible, return IDX.

4. if the type of the IDX-th ``bbm::arg`` is unique, and there is a matching
   unique type in TUP, then return the index (in TUP).

5. else: fail (return index outside range).
