std::tuple extensions
=====================

BBM heavily relies on ``std::tuple``, and for your convenience some additional
extensions to the ``stl`` library are included:

1. Create a tuple from other types that support std::get:

   .. doxygenfunction:: bbm::to_tuple

2. Get a subset of a tuple:

   .. doxygenfunction:: bbm::subtuple
   
3. Handling creating tuples to references, and removing references:

   .. doxygenfunction:: bbm::make_ref_tuple

   .. doxygenfunction:: bbm::value_copy_tuple

4. Flatten recursive tuples:
      
   .. doxygenfunction:: bbm::tuple_flatten

5. Adding and removing ``const`` to tuple types:
   
   .. doxygenfunction:: bbm::tuple_add_const

   .. doxygenfunction:: bbm::tuple_remove_const

BBM also includes a few As well as additional type-traits:

.. doxygentypedef:: bbm::to_tuple_t
   
.. doxygentypedef:: bbm::tuple_cat_t

.. doxygentypedef:: bbm::subtuple_t

.. doxygentypedef:: bbm::tuple_flatten_t

.. doxygentypedef:: bbm::tuple_add_const_t

.. doxygentypedef:: bbm::tuple_remove_const_t

And finally, BBM support ostream forwarding of ``std::tuple``.
