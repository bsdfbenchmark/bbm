bbm::string_converter
=====================

We already saw that one can override the serialization/deserialization
behavior by adding a ``toString`` and a static ``fromString`` method to a
bsdfmodel.  However, this is not limited to only bsdfmodels and any class can
add serialization/deserialization support by adding these two methods.  

However, in some cases one might not access to the class declaration.  BBM
offers an addition method for adding serialization support through a
sepecialzation of the ``bbm::string_converter`` class:


.. doxygenstruct:: bbm::string_converter
   :members:
   :undoc-members:

.. note::

   Core ``string_converter`` specialization are defined in
   `include/core/stringconvert.h
   <../doxygen/html/include_2core_2stringconvert_8h_source.html>`_. However,
   any method that implements a specialization should not directly include
   this header file, but instead include `concepts/stringconvert.h
   <../doxygen/html/include_2concepts_2stringconvert_8h_source.html>`_.
      
To aid in parsing strings, bbm includes the following helper methods defined
in `include/util/string_util.h <../doxygen/html/string__util_8h_source.html>`_:

.. doxygenfunction:: bbm::string::remove_whitespace

.. doxygenfunction:: bbm::string::remove_brackets

.. doxygenfunction:: bbm::string::remove_comment

.. doxygenfunction:: bbm::string::get_keyword

.. doxygenfunction:: bbm::string::split_eq

.. doxygenfunction:: bbm::string::split_args


