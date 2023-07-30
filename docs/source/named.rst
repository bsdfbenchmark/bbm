bbm::named
==========

One use of ``bbm::string_literal`` is to create `named` version of any type
that supports ``std::get`` (e.g., ``std::tuple``, ``std::pair``,
``std::array``, etc...).

.. code-block:: c++

   bbm::named<std::tuple<int, float, char>, "a", "b", "c"> foo(1, 2.0, 'b');
   std::cout << foo << std::endl;

The above example will create a 3-element tuple, and associate the name
(``string_literal``) "a" to the first element, "b" to the second, and "c" to
the last.  The purpose of named tuples is two fold:

1. tuples are often used to return multi-value results from a function.
   However, a tuple requires that the programmer remembers the role of each
   element in the tuple, which can be error prone and might require the
   programmer to read the function implementation.  Named tuples offer a
   convenient way to store a descriptive name with each tuple element.

2. ``C++`` does not allow for inline definition of a struct parameter or
   return type in a function declaration, requiring the struct to be defined
   separately.

Query
-----

There several methods to query a element from a named tuple:

.. code-block:: c++

   std::cout << std::get<0>(foo) << std::endl;   // '1'
   std::cout << bbm::get<"a">(foo) << std::endl; // '1'

A compile error is throws if the requested name does not exist in the
structure!  Lookup and storage of names all occurs at compile time and there
is no runtime overhead.  One can query the type with the ``value_type``
typedef and the values of the ``value_type`` with ``values``:

.. code-block:: c++

   std::cout << bbm::typestring< decltype(foo)::value_type > << std::endl;
   std::cout << foo.values() << std::endl;

Named tuples also support structured bindings: 

.. code-block:: c++

   auto [a1, b1, c1] = foo;
   std::cout << c1 << std::endl;  // 'b'

   auto [a2, c2] = bbm::pick<"a", "c">(foo);
   std::cout << c2 << std::endl;  // 'b'

as well as tying existing variables by name:

 .. code-block:: c++

    int a;
    float b;
    char c;
    bbm::tie<"a", "b", "c">{a,b,c} = foo;

Note that for both ``pick`` as well as ``tie`` the order of the names does not
need to correspond to the order in which they are defined. One can even repeat
a name.

Query names
-----------

named types allow to check if a name is present, or to query a name:

.. code-block:: c++

   std::cout << foo::has_name<"d"> << std::endl;  // false
   std::cout << foo:names << std::endl; // yields a tuple: ("a", "b", "c")
   std::cout << foo::template name<0> << std::endl; // yields "a" (use foo:size to require the length)

bbm::make_named
---------------

A helper function is included to simplify creation of named tuples:

.. code-block:: c++

   auto n1 = bbm::make_named<"a", "b", "c">( std::make_tuple(1, 2.0, 'b') );
   auto n2 = bbm::make_named<"a", "b", "c">(1, 2.0, 'b');

In the former also works for other types that support ``std::get``.

Recursive named types
---------------------

Named types can be recursive, and ``bbm::get`` supports recursive retrieval:

.. code-block:: c++

   auto n3 = bbm::make_named<"a">( bbm::make_named<"b", "c">('b', 'c') );

   std::cout << n3 << std::endl;   // (a = (b = 'b', c = 'c'))
   std::cout << bbm::get<"a">(n3) << std::endl;  // (b = 'b', c = 'c')
   std::cout << bbm::get<"b">( bbm::get<"a">( n3) ) << std::endl; // 'b'
   std::cout << bbm::get<"a", "b">( n3 ) << std::endl;  // 'b'

Care must be taken when constructing recursive named types:

.. code-block:: c++

   bbm::named< bbm::named< std::tuple<int,int>, "a", "b">, "c", "d" > A{1, 2};
   bbm::named< std::tuple< bbm::named< std::tuple<int,int>, "c", "d" >, "e" > B(A);

The first does not yield a recursive named type, it simply replaces the names
of the inner named type with those of the outer one (i.e., ``(c = 1, d =
2)``).  The latter is a true recursive named type yielding ``(e = (c = 1, d =
1))``.

Type traits
-----------

Shorthand typedefs of named tuples with 2, 3, and 4 elements exist:

.. doxygentypedef:: bbm::named2

.. doxygentypedef:: bbm::named3

.. doxygentypedef:: bbm::named4

To check if a type is a named type, you can use ``bbm::is_named_v<T>``.  To check
if two named types have the same set of names (but possibly in different
order): ''bbm::named_equivalence_v<T, U>''.

To remove the names from a type:

.. doxygenfunction:: bbm::anonymize_v

.. doxygentypedef:: bbm::anonymize_t


Additional Operations
---------------------

Concat
~~~~~~

.. doxygenfunction:: bbm::named_cat

Corresponding type-trait:
                     
.. doxygentypedef:: bbm::named_cat_t

Prefix and Postfix
~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: bbm::prefix_names

.. doxygenfunction:: bbm::postfix_names

Corresponding type-traits:

.. doxygentypedef:: bbm::prefix_names_t

.. doxygentypedef:: bbm::postfix_names_t

Flatten
~~~~~~~

.. doxygenfunction:: bbm::named_flatten

.. doxygenfunction:: bbm::merge_named_flatten

