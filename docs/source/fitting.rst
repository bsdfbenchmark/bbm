.. warning::

   BSDF fitting is still experimental, not fully validated. and the interface
   is not yet stable.

BBM has been designed with BSDF fitting in mind. BSDF fitting is the process
where the parameters of a BSDF model is optimized such that some error metric
between the fitted model and a reference model are minimized.

Basic Usage
===========

Consider the following example that fits a Cook-Torrance BSDF model to the
MERL-MIT measured ``Alum-Bronze``:

.. code-block:: c++

  #include <iostream>
  #include "bbm.h"
  using namespace bbm;

  #include "optimizer/compass.h"
  #include "loss/cosine_weighted_log.h"

  int main(int argc, char** argv)
  {
    BBM_IMPORT_CONFIG( floatRGB );
  
    size_t maxItr = 100;

    // create model to fit and reference to fit to.
    auto fitted = aggregate( lambertian<Config>(), cooktorrance<Config>() );
    merl<Config> reference("alum-bronze.binary");

    // Use a standard Log loss
    standardLog loss(fitted, reference, {90, 30}, {1, 9});
    auto param = parameter_values(fitted);

    std::cout << "Initial param: " << param << std::endl;
    std::cout << "Initial loss : " << loss() << std::endl;

    // Use the bsdfmodel defined box constraints on the parameters
    auto low = parameter_lower_bound(fitted);
    auto up = parameter_upper_bound(fitted);

    std::cout << "Lower: " << low << std::endl;
    std::cout << "Upper: " << up << std::endl;

    // Use a compass search for optimization
    compass opt(loss, param, low, up);

    // optimize for 100 steps, or until convergence
    for(size_t t=0; t < maxItr && !bbm::all(opt.is_converged()); ++t)
    {
      auto err = opt.step();
      std::cout << err << ": " << param << ", " << opt.is_converged() << std::endl;
    }

    return 0;
  }

This example demonstrates the three key components in BSDF fitting in BBM:

1. the ability to enumerate the BSDF parameters in a vector-like structure
   using the ``parameter_values`` method, as well as the ability to query the
   upper and lower limits of the parameters.  The enumerated parameter values
   have been decoupled from the BSDF model making generic interfacing with
   optimization algorithms easier.

2. Decoupling of the fitting from the optimization algorithm.  Many non-linear
   optimization implementations assume a L2 error metric.  Decoupling the loss
   from the optimization procedure allows for experimentation.

3. Separation of the optimization algorithm from the BSDF fitting.  The above
   code employs a ``compass`` search. However, future developments can include
   other algorithms such as Adam.


Parameter Enumeration
=====================

Parameters can be enumerated by:

.. doxygenfunction:: bbm::parameter_values(MODEL &&, bsdf_attr)

Note a version of ``parameter_values`` exists for both ``bsdfmodel`` and
``bsdf``.  By changing the flags, a subset of the parameters can be
enumerated, for example to separate the fitting of albedo and the non-albedo
parameters:

.. doxygenenum:: bbm::bsdf_attr

The ''DiffuseScale'' and ``SpecularScale`` refer to albedo parameters. E.g.,
the ``albedo`` added by wrapping a ``bsdfmodel`` in a ``scaledmodel`` is a
scale.  ``Scalar = DiffuseScale | SpecularScale``. The ``DiffuseParameter``
and ``SpecularParameter`` represent all non-albedo parameters, with
``Parameter = DiffuseParameter | SpecularParameter``.  ``Diffuse`` and
``Specular`` is the union of the corresponding scale and parameter.  Finally,
a parameter marked as ``Dependent`` is not directly optimizable; it is assumed
that its value dependents directly or indirectly on the values of other
parameters.
                 
A similar interface exists for enumerating ``parameter_default_values``,
``parameter_lower_bound``, and ``parameter_upper_bound``.


Fitting Metric
==============

Similar to the relation between ``bsdfmodel`` and ``bsdf``, BBM splits a
fitting metric in a ``lossfunction`` and a ``loss`` (with corresponding
``loss_base`` and ``loss_ptr``).  The latter wraps a ``lossfunction`` and
provides an interface with virtual function calls. 

A ``lossfunction`` must meet ``bbm::concepts::lossfunction``:

.. doxygenconcept:: bbm::concepts::lossfunction

The loss function constructor should precompute any values that do not change
during the whole optimization process.  The ``init`` method should precompute
any values that remain constant within an optimization step.  Finally,
``operator()`` should return the loss.  Note that ``init`` is only called once
per optimization step, whereas ``operator()`` might be called multiple times.
A (fictional) example of how this could be used: during construction the loss
might precompute cosines of angles that are evaluated as part of the loss; the
init function could preselect a batch of directions to compute the loss over;
and the ``operator()`` might sum the differences over the batch and scale it
by the precomputed cosine.

Many fitting metrics compute a loss over a number of sampled ``(in,out)``
directions. So simplify creation, BBM introduces a ``sampledlossfunction``
(which also meets ``bbm::concepts::sampledlossfunction``):

.. doxygenstruct:: bbm::sampledlossfunction
   :members:
   :undoc-members:

Sampled Loss Function
---------------------
      
The behavior of a ``sampledlossfunction`` is determined by a
sample loss function that computes the loss over a single sample and which
must meet ``bbm::concepts::samplelossfunction``:

.. doxygenconcept:: bbm::concepts::samplelossfunction

A number of sampled loss functions are predefined, that will be used in
conjunction with a linearizer_ to create different `loss functions`_.

Cosine weighted L2
~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: bbm::nganL2_error

.. doxygenstruct:: bbm::lowL2_error

.. doxygenstruct:: bbm::bieronL2_error

Cosine weighted Log
~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: bbm::standardLog_error

.. doxygenstruct:: bbm::lowLog_error

.. doxygenstruct:: bbm::bieronLog_error                      
   

Linearizer
----------

The behavior of a ``sampledlossfunction`` is also determined by a linearizer
(``bbm::concepts::inout_linearizer``) that determines how samples are
generated:

.. doxygenconcept:: bbm::concepts::inout_linearizer

An example of a linearizer is ``spherical_linearizer`` that samples the
incident and outgoing hemisphere of directions:

.. doxygenstruct:: bbm::spherical_linearizer
   :members:
                   
.. note::

   A ``inout_linearizer`` is also a useful tool for creating a
   ``staticmodel``.  The ``merl`` static BSDF model utilizes a
   ``merl_linearizer``.

Loss functions
---------------
   
The ``spherical_linearizer`` is used to predefine a number of sampled loss
functions (stored in ``include/loss``.

Sampled Squared Loss
~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: bbm::nganL2
   :outline:
      
.. doxygenstruct:: bbm::lowL2                   
   :outline:
                   
.. doxygenstruct:: bbm::bieronL2
   :outline:
                   
Sampled Log Loss
~~~~~~~~~~~~~~~~
                   
.. doxygenstruct:: bbm::standardLog
   :outline:
      
.. doxygenstruct:: bbm::lowLog                   
   :outline:
      
.. doxygenstruct:: bbm::bieronLog
   :outline:
                    
Optimizer
=========

The optimization algorithms interface is defined by
``bbm::concepts::optimization_algorithm``

.. doxygenconcept:: bbm::concepts::optimization_algorithm

Similar to ``bsdfmodel`` and ``bsdf``, a virtual interface is provided through
``optimizer`` (and similarly ``optimizer_base`` and ``optimizer_ptr``).

An example of an optimization algorithm is ``bbm::compass``:

.. doxygenstruct:: bbm::compass
   :members:

.. note::

   optimization algorithms are defined directly in the ``bbm`` namespace and
   stored in ``include/optimizer``

When implementing a new optimization algorithm, care must be taken to ensure
that the implementation is compatible with packet types.


Fit files
=========

To store and exhange BSDF fits, BBM uses a simple text based format.  In this
format lines with comments start with ``#``.  Each material fit is stored as:
``name = model(parameters``, where ``name`` is a unique identifier (e.g., the
material name), ``model(parameters)`` is what you would see when the ``str``
method in Python is called on the model or the output of ``bbm::toString``.
We refer the ``fits`` subdirectory for examples.  To import the BSDF fits in
python:

.. code-block:: python

   import bbm_floatRGB as bbm
   from import_fits import *
   fits = import_fits('fits/ngan_ward.fit', bbm)
   str(fits['alum-bronze'])

The 3rd line, reads the ``fits/ngan_ward.fit`` file, and imports the fits into
a dictionary (called ``fits``).  This dictionary contains the BSDF fits with
the key the name of the MERL material, and the value the corresponding BSDF
(defined in the Python ``bbm`` namespace).

In ``C++`` we can use the ``bbm::io::importFIT`` method:

.. doxygenfunction:: bbm::io::importFIT

For example:

.. code-block:: c++

   #include "io/fit.h"

   // ....
   
   std::map<std::string, bsdf_ptr<Config>> fits;
   bbm::io::importFIT("filename.fit", fits);
   std::cout << fits["alum-bronze"] << std::endl;

Similarly, one can export an ``std::map`` of fits using the ``bbm::exportFIT``
method:

.. doxygenfunction:: bbm::io::exportFIT

