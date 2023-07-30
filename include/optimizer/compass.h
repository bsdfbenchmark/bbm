#ifndef _BBM_COMPASS_H_
#define _BBM_COMPASS_H_

#include <limits>

#include "concepts/optimization_algorithm.h"
#include "concepts/lossfunction.h"
#include "concepts/parameter.h"

/************************************************************************/
/*! \file compass.h

  \brief Compass search (i.e., pattern search) optimization.  Gradient free
  optimization algorithm that probes each cardindal direction.

  Follows the algorithm on page 402 from: "Optimization by Direct Search: New
  Perspectives on Some Classical and Modern Methods" [Kolda et al. 2003]:
  https://doi.org/10.1137/S003614450242889
  
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Compass Search

    \tparam LOSSFUNC = loss function
    \tparam PARAM = parameters to optimize
    \tparam BOX = box constraint type (default=PARAM)
    
    Performs a pattern search by probing each cardinal direction in the
    parameter space.

    When the Value type is a packet, this compass search will perform 
    N parallel searches (where N == size of the packet).

    Satisfies: concepts::optimization algorithm
  ***********************************************************************/
  template<typename LOSSFUNC, typename PARAM, typename BOX=PARAM> requires concepts::lossfunction<LOSSFUNC> && concepts::parameter<PARAM> && concepts::parameter<BOX>
    struct compass
  {
    BBM_IMPORT_CONFIG( LOSSFUNC );

    /********************************************************************/
    /*! \brief Constructor: compass search

      \param lossfunc = loss function to minimize
      \param param = set of parameters to optimize
      \param lower = lower bound of box constraint
      \param upper = upper bound of box constraint
      \param tolerance = threshold on step size to determine convergence (default=Epsilon)
      \param stepSize = initial step size (default=1.0)
      \param contraction = reduction factor for step size if no probe in the cardinal directions improves the loss (default=0.5)
      \param expansion = expansion factor for step size if a good candinal direction was found (default=1.0)
     *********************************************************************/
    compass(LOSSFUNC& lossfunc, PARAM& param, const BOX& lower=BOX(), const BOX& upper=BOX(),
          Scalar tolerance=Constants::Epsilon(), Scalar stepSize=1.0, Scalar contraction=0.5, Scalar expansion=1.0, Mask mask=true) :
            _lossfunc(lossfunc),
            _param(param), _lower(lower), _upper(upper),
            _initialStep(stepSize),
            _tolerance(tolerance),
            _contraction(contraction), _expansion(expansion),
            _mask(mask)
    {
      // Initialize cardindal directions
      for(Scalar i=1; i <= std::size(param); ++i)
      {
        _directions.push_back(+i);
        _directions.push_back(-i);
      }
      
      // reset the internal state
      reset();
    }

    /********************************************************************/
    /*! \brief probe each cardinal direction, and update the parameters to
       the one with the lowest loss.

       \returns loss after update
    *********************************************************************/
    Value step(void)
    {
      Value best = 0;
      Value loss = _lossValue;

      // lambda to apply a candinal direction update of magnitude _step in _param
      auto probe = [&](auto cardinal, Mask mask=true)
      {
        mask &= (cardinal != 0);  // exclude invalid cardinal directions;
        auto index = bbm::cast<index_t<Value>>( bbm::abs(cardinal) - 1 );
        Value value = lookup<Value>(_param, index, bbm::cast<index_mask_t<Value>>(mask)) + bbm::select(cardinal < 0, -_step, _step);
        set(_param, index, value, mask);
      };

      // only update nonconverged
      Mask optimize = _mask && !is_converged();

      // Quick bailout
      if(bbm::none(optimize)) return 0;
      
      // Update loss internal state
      _lossfunc.update();

      // for each cardinal direction
      for(const auto& cardinal : _directions)
      {
        // move parameter in cardinal direction
        probe(cardinal, optimize);

        // check parameter falls inside box constraints (if any)
        Mask in_box = optimize;
        if(_lower != BOX() || _upper != BOX())
        {
          multirange_for([&](const auto& param, const auto& lower, const auto& upper)
          {
            in_box &= (param >= lower) && (param <= upper);
          }, _param, _lower, _upper) ;
        }

        // compute loss
        auto err = _lossfunc(in_box);

        // remember if better
        best = bbm::select(in_box && (err < loss), cardinal, best);
        loss = bbm::select(in_box && (err < loss), err, loss);

        // restore _param
        probe(-cardinal, optimize);
      }

      // if loss decreases; make permanent change, otherwise decrease step
      optimize &= (loss < _lossValue);
      probe(best, optimize);
      _step = bbm::select(optimize, _expansion*_step, _contraction*_step);
      _lossValue = bbm::select(optimize, loss, _lossValue);
      
      // Done.
      return _lossValue;
    }

    /********************************************************************/
    /*! \brief reset the step size
     ********************************************************************/
    void reset(void)
    {
      // reset cardinal directions
      _step = _initialStep;

      // compute current loss
      _lossfunc.update();
      _lossValue = _lossfunc(_mask);
    }

    /********************************************************************/
    /*! \brief is_converged when _step < _tolerance
     ********************************************************************/
    Mask is_converged(void) const
    {
      return (_step < _tolerance) || (_mask == false);
    }
    
    /////////////////////
    // Class Attributes
    /////////////////////
  private:
    // parameters, constraint, and loss function
    LOSSFUNC& _lossfunc;
    PARAM& _param;
    BOX _lower, _upper;

    // optimization state
    Value _step;
    Value _lossValue;
    
    // optimization config
    std::vector<Scalar> _directions;
    Scalar _initialStep;
    Scalar _tolerance;
    Scalar _contraction;
    Scalar _expansion;
    Mask _mask;
  };

  BBM_CHECK_CONCEPT( concepts::optimization_algorithm, compass<lossfunction<>, parameter<>> );
  
} // end bbm namespace

#endif /* _BBM_COMPASS_H_ */
