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
typedef and the values of the ``value_type`` with the ``values()`` method:

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
   std::cout << foo::template find_name<"b"> << std::endl; // 1

Note ``find_name`` will return the position of the named in the ``value_type``
structure.  It will return a position beyond the last element if the name does
not exist (i.e., ``has_name == false``).
   
bbm::make_named
---------------

A helper function is included to simplify creation of named tuples:

.. code-block:: c++

   auto n1 = bbm::make_named<"a", "b", "c">( std::make_tuple(1, 2.0, 'b') );
   auto n2 = bbm::make_named<"a", "b", "c">(1, 2.0, 'b');

In the former also works for other types that support ``std::get``; the latter
always creates a named tuple.

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

To check if a type is a named type, you can use ``bbm::is_named_v<T>``.  To check
if two named types have the same set of names (but possibly in different
order): ''bbm::named_equivalence_v<T, U>''.

To remove the names from a type (also works on non-named types, i.e., it
returns the non-named type unaltered):

.. doxygenfunction:: bbm::anonymize_v

.. doxygentypedef:: bbm::anonymize_t


Additional Operations
---------------------

Additional methods on named types are defined in `util/named_util.h
<../doxygen/html/util_2named_8h_source.html>`_.

Copy by value
~~~~~~~~~~~~~

This method mirrors the corresponding method defined on tuples (defined in
`util/tuple.h
<../doxygen/html/build/docs/html/doxygen/html/tuple_8h_source.html>`_, and it
creates a copy of a named tuple by value (i.e., references are dereferenced):

.. doxygenfunction:: bbm::value_copy_named

Corresponding type-trait:

.. doxygentypedef:: bbm::value_copy_named_t


Concat
~~~~~~

.. doxygenfunction:: bbm::named_cat

Corresponding type-trait:
                     
.. doxygentypedef:: bbm::named_cat_t

                    
Subset
~~~~~~

.. doxygenfunction:: bbm::subnamed

Corresponding type-trait:

.. doxygentypedef:: bbm::subnamed_t
                    
                    
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


Sorting
~~~~~~~

BBM offers the following compile-time methods for sorting and checking is a
named type is sorted:

.. doxygenvariable:: bbm::is_named_sorted_v

.. doxygenfunction:: bbm::sort_named

Run-time search
~~~~~~~~~~~~~~~

The typedef ``find_name`` on a named type allows to perform a linear search
for a given name at compile-time.  If the named type is sorted, then one can
also perform the same search using a binary search:

.. doxygenvariable:: bbm::binary_search_named_v

In some case, the target name might not be known at compile time, and be
provided at run-time as an ``std::string``.  BBM offers tools for run-time
searching (linear is not sorted or binary if sorted):

.. doxygenfunction:: bbm::linear_search_named
   :outline:
                     
.. doxygenfunction:: bbm::binary_search_named
   :outline:

Both methods have a similar signature. Searching in a potentially heteroegeous
data structure (e.g., tuples) imposes a few constraints:

1. returning an index to the found element is not helpful.  One can querry an
   element by its index in a tuple using ``std::get``. However, the index must
   be known at compile time.

2. because the element might all have different types in a named type,
   returning the element itself is not an option either.

Therefore, both methods take a lambda function that takes an index as a
template parameter, a reference to the string, named type and any number of
'context' arguments. The latter are used to pass any information to each
call. These same context variables are also passed to the search method and
passed without modification to the lambda:

.. code-block:: c++

   auto n = bbm::make_named<"a", "b", "c">(1.0, 'b', 3);
   bbm::binary_search_named( "c", n, []<size_t IDX>(const std::string& str, auto&& naned_type, float ctx) { std::cout << IDX << ", " << ctx << std::endl; }, 123.45);

The above code will output '3, 123.45' (the position of the search named ("c")
and the value of the context variable.  Optionally, the lambda may return a
value as long as it returns the same type for each possible IDX value.

In case of a binary search, the lambda is called with the index of where the
element should be added in the named type without breaking the sort.  It is
the responsibility of the programmer to check:

1. if an exact match is required, that the name of the corresponding element
   has the correct name:

   .. code-block:: c++

      std::get<IDX>(std::forward<decltype(named_type)>(named_type)) == str

2. if the returned index falls within the bounds of the named type; it is
   possible that the requested name would be inserted at the end:

   .. code-block:: c++

      IDX < std::tuple_size_v<decltype(named_type)>

In case of a linear search, if the name is not found, index is set to the size
of the type (case 2 above).

.. warning::

   The generated code will create an instance of the lambda method for *each*
   possible index value!  This is the case for both the linear and binary
   search.  Hence, if a binary search is performed on a named tuple with 32
   elements, then 33 calls (one for each element plus one for first invalid
   index value) are generated.  Hence, it is advisable to keep the lambda
   function as compact as possible.

   
