Precomputation
==============

Certain BSDF models rely on precomputed data.  BBM strives to include
precomputation code to generate such data.  Precomputation code is placed in
the ``precompute`` subdirectory.  Each Precomputation contains its own
``CMakeLists.txt`` which is automatically included by BBM's own
``CMakeLists.txt`` script.   The compile target should be added to the
``precompile`` target at the end of the precompute ``CMakeLists.txt``:

.. code-block:: cmake

  add_dependencies(precompute ${NAMELIST})


The precompute ``CMakeLists.txt`` is also responsible for ensuring that the
file is not recompiled if the header file already exists when running
``cmake``.

Generated header files that store precomputed data should be stored in
``include/precompute`` and its code should be placed on the
``bbm::precomputed::<custom namespace>`` namespace.

Ideally, precomputed data should be stored such that it can be directly
initialized with ``std::initializer-list``.  One such standard structure is
``std::array``.  However, an ``std::array`` suffer from the following
problems:

1. The relation between the indices of the array and the parameter it maps to
   are implicit.  The programmer needs to remember this an perform the
   conversion before lookup.  This is error prone.

2. Often values need to be interpolated.  This again can result in code
   duplication of interpolation code.

To resolve this, BBM offers a helper structure for storing precomputed data:

.. doxygenstruct:: bbm::tab
   :members:

Simple precompute example
-------------------------

.. code-block:: c++

   const bbm::tab<float, std::array{4},
                  decltype( [](const auto& p) { return p*3.0; } )
      > precomp = { 0.0, 0.33, 1.0, 3.0 };

   std::cout << precomp.lookup(0.3) << std::endl;
   std::cout << precomp.lookup(1.3) << std::endl;
   std::cout << precomp.interpolate(0.3) << std::endl;


This example maps a parameter value from the 0 to 1 range to an index between
0 to 3, and reads out the corresponding value.  The output of the above code
is:

.. code-block:: none

   (value = 0.330000, valid = 1)
   (value = 3.000000, valid = 0)
   (value = 0.464000, valid = 1)

The result is a named tuple where the first element is the result and the 2nd
value is a ''boolean'' indicating whether or not the query produced a valid
index (the lookup is always clamps to the end of the data structure to avoid
reading outside the bounds of the stored data).

The index computation is performed by the lambda function stored in the
``tab`` declaration.  In this case, the coefficient ``p`` is converted to an
index by scaling it by ``3.0``. The ``lookup`` method returns the nearest
neighbor to the (integer casted) index.  ``interpolate`` will perform a
multi-linear interpolation.

Named tuples are a key component in BBM. Basic operations are:

.. code-block:: c++

   auto pc = precomp.lookup(0.3);
   auto value = bbm::get<"value">(pc);  // value = 0.330000

   auto [val, mask] = pc; // val = 0.33000, mask = true

   auto [m, v] = bbm::pick<"valid", "value">(pc); // m = true, v = 0.330000
   
Note that when using ``pick`` that the order of the names to lookup does not
need to correspond to the order in the named tuple, nor does every name need
to be retrieved.  In general, named lookups (i.e., ``bbm::get`` and
``bbm::pick``) are preferred since they make it clear what is looked up, plus
it is robust to changing the order or adding elements to the tuple.


Simple multi-dimensional example
--------------------------------

``bbm::tab`` can also easily support multi-dimensional precomputed tables:

.. code-block:: c++

   const bbm::tab<float, std::array{2, 4},
                  decltype( [](const auto& x) { return x; } ),
                  decltype( [](const auto& y) { return y * 3.0; } )
      > precomp = {
        0.0, 0.3, 1.0, 3.0,
        1.0, 1.0, 1.0, 3.0 };

   std::cout << precomp.lookup(0.2, 0.4) << std::endl;
   std::cout << precomp.interpolate(0.2, 0.4) << std::endl;

yields:

.. code-block:: none

   (value = 0.300000, valid = 1)
   (value = 0.552000, valid = 1)

where lookup directly reads the element at (0,1), and interpolate will perform
a bilinear interpolation (between (0,1), (0,2), (1,1), and (1,2)).

.. note::

   Internally ``bbm::tab`` stores the data in a linearized 1D ``std::array``.


Correlated coordinates
----------------------

The above examples map a single coordinates to a single index.  However,
sometimes the mapping to an index depends on multiple coordinates.

.. code-block:: c++

   const bbm::tab<float, std::array{2, 4},
                  decltype( [](const auto& x, const auto& y) { return x+y; } ),
                  decltype( [](const auto& x, const auto& y) { return y; } )
     > precomp = {
       00, 01, 02, 03,
       10, 11, 12, 13};

   std::cout << precomp.lookup(0,1) << std::endl; // reads out (0+1, 1) (valid)
   std::cout << precomp.lookup(1,2) << std::endl; // reads out (1+2, 2) (invalid)

The number of coordinates passed to the lambda functions must be the same for
all lambda functions and either be 1 (simple lookup) or equal to the number of
dimensions (2 in this case).

Complex precomputed types
-------------------------

In the above examples the value type was ``float``. However, more complex
types are possible too:

.. code-block:: c++

   const bbm::tab< bbm::named<std::tuple<float, float>, "a", "b">, std::array{4}> precomp
      = {{{ {0,1}, {0,2}, {1,1}, {2,2} }}};

   std::cout << precomp.lookup(2) << std::endll  // (value = (a = 1.0000, b = 1.0000), valid = 1)
   std::cout << bbm::get<"value">( precomp.lookup(0) ) << std::endl;  // (a = 0.0000, b = 1)
   std::cout << bbm::get<"value", "b">( precomp.lookup(1) ) << std::endl; // 2

In this the first lookup return the named tuple ("value" and "valid") with
"value" yielding another named tuple with two elements "a" and "b".  The
second lookup we only return the named field "value".  The last lookup does a
recursive get, where we return the field "b" in the field "value".
   
.. warning::

   It is recommended to only use trivially constructible types such as
   ``std::array``, ``std::tuple`` or named tuples as in the above example.  If
   not, compilation might fail or be *very* slow.

