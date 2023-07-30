std::tuple extensions
=====================

BBM heavily relies on ``std::tuple``, and for your convenience some additional
extensions to the ``stl`` library are included:

1. Handling creating tuples to references, and removing references:

   .. doxygenfunction:: bbm::make_ref_tuple

   .. doxygenfunction:: bbm::value_copy_tuple

2. Flatten recursive tuples:
      
   .. doxygenfunction:: bbm::tuple_flatten

3. Adding and removing ``const`` to tuple types:
   
   .. doxygenfunction:: bbm::tuple_add_const

   .. doxygenfunction:: bbm::tuple_remove_const

BBM also includes a few As well as additional type-traits:

.. doxygentypedef:: bbm::tuple_cat_t

.. doxygentypedef:: bbm::tuple_flatten_t

.. doxygentypedef:: bbm::tuple_add_const_t

.. doxygentypedef:: bbm::tuple_remove_const_t

And finally, BBM support ostream forwarding of ``std::tuple``.
