Pointers and References
=======================

bbm::pointer
------------

STL's shared pointers are important tools for reducing memory leaks.  In some cases,
a pointer type is needed that can handle both shared and raw pointers at
run-time.  The ``bbm::pointer`` class provides this functionality by abusing
the deleter in ``sdt::shared_ptr``:

.. doxygenclass:: bbm::pointer
   :members:
   :undoc-members:

bbm::reference
--------------

STL provides a ``std::reference_wrapper`` to wrap provide a reference to an
object (essentially, it wraps a pointer to the object).  However,
``std::reference_wrapper`` has a few shortcomings:

1. the reference cannot refer to nothing.  While this seems like a good thing
   (avoiding ``null_ptr``). It can cause issues when creating arrays of
   references that are iteratively initialized.

2. ``std::reference_wrapper`` cannot handle rvalues, where as regular
   references can:

   .. code-block:: c++

      const float& a = 4.0;

   In the above example, a temporary float is allocated to which ``4.0`` is
   assigned, and which is deleted when ``a`` goes out of scope.

BBM offers a reference wrapper: ``bbm::refernce`` / ``bbm::const_reference``
that supports the former, and ``bbm::persistent_reference`` /
``bbm::const_persistent_reference`` that support both.  Note, that for
non-const references the latter is equivallent to the former.

``bbm::reference`` will 'bind' to the first value/variable assigned to it.  If
the ``bbm::reference`` is cast to the underlying type before assignment, then
a ``bbm_unassigned_ref`` exception is thrown.

.. code-block:: c++

   float a = 3.0;
   bbm::reference<float> r = a;
   bbm::persistent_reference<const float> p = 4.0;
   bbm::reference<float> e;
   
   std::cout << r << std::endl;  // '3.0'
   std::cout << p << std::endl;  // '4.0'
   std::cout << e << std::endl;  // throws bbm_unassigned_ref

Application: bbm::vector
~~~~~~~~~~~~~~~~~~~~~~~~

``bbm::vector`` is an extension to ``std::vector`` that transparently supports
references (using ``bbm::reference`` and ``bbm:cast_itr`` (which casts the
``bbm::refernce`` to a C++ reference when iterating over the vector)).
``bbm::vector`` also include ostream forwarding as well as basic math
operations (addition, subraction, division, and multiplication).

.. code-block:: c++

   float a = 1.0f;
   float b = 2.0f;
   bbm::vector<float&> v = {a, b};

   std:cout << v << std::endl;  // [1.0, 2.0]
   v[0] = 10.f;
   std::cout << a << std::endl; // 10.0
   
``bbm::vector<Value&>`` is the underlying type when enumerating the attributes
of a BSDF model with ``bbm::parameter_values``.
