Future Plans
============

BBM is a work in progress. Here we list the plans for the next immediate
versions (subject to change) as well as unscheduled plans for longer term
changes:


Version 0.5.2
-------------

+ Improvements to Python interface for passing parameters to differentiable and packet types.
+ Packet support for DRJIT
+ Better Python support for DRJIT
  
  
Version 0.5.3
-------------

+ Holzschuch and Pacaknowski's `two-scale microfacet reflectance model
  <https://doi.org/10.1145/3072959.3073621>`_

+ The 'Dinsey BSDF model
  <https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf>_

  
Version 0.5.4
-------------

+ More robust auto-diff support with DRJIT and Enoki

+ Gradient based BSDF fitting

  
Version 0.6.0
-------------

+ Spatially varying BSDFs

+ Improved vectorization support

Version 0.7.0
-------------

+ BSDF fitting strategies: implementation of various published fitting strategies

Not yet scheduled/wish-list
---------------------------

+ Improved Python support; allow for Python BSDF

+ DRJIT/Enoki GPU support
  
+ Mitsuba 2/3 exporter

+ PyTorch bindings

+ PBRT exporter

+ Support for other measured static BSDF models

+ Multi-spectral support

+ Data-driven masking and shadowing for NDFs with no analytical solution

+ Data-driven reflectance for BSDF models without an anaitical solution
