bbm::string_literal
===================

A string literal (``include/util/string_literal.h``) is a compile-time ``constexpr`` string implementation.
It is used extensively in BBM to pass constexpr strings as a template literal.

.. code-block:: c++

   template<string_literal STR>
     void foo(void)
   {
     std::cout << STR << std::endl;
     std::cout << toTypestring( STR ) << std::endl;
   }

   foo<"TEST">();

The above example will print out "TEST" and indicate that STR has type ``const
bbm::string_literal<5>``.  String literals support basis operations such as
concatenation, comparison, and casting to a regular (zero terminated) C string:

.. doxygenstruct:: bbm::string_literal
   :members:
   :undoc-members:

A custom literal ``sl`` exists which is needed when the template literal is
defined as ``auto``.

.. code-block:: c++

   template<auto STR>
     void bar(void)
   {
     std::cout << STR << std::endl;
     std::cout << toTypestring( STR ) << std::endl;
   }

   bar<"TEST"_sl>();  // just passing "TEST" will cause a compile error.

A type trait ``bbm::is_string_literal_v`` has been defined to detect
string literals (e.g., when using ``auto`` as template type).

Finally, ``include/util/to_string_literal.h" contains a ``constexpr``
implementation to convert a (integer) literal to a ``string_literal``:

.. doxygenfunction:: bbm::to_string_literal


   
