#ifndef _BBM_CDF_H_
#define _BBM_CDF_H_

#include <numeric>
#include <algorithm>

#include "util/named.h"
#include "util/iterator_util.h"

/************************************************************************/
/*! \file cdf.h

  \brief Discrete Cummulative Distribution Function (cdf) constructed from a
  set of samples and accompanying sampling method.
*************************************************************************/
  
namespace bbm {

  /**********************************************************************/
  /*! \brief CDF data structure

    \tparam C = container type to store CDF as well as to provide samples for
               construction.

    Requires that the container 'C' is a range.
  ***********************************************************************/
  template<typename C> requires std::ranges::range<C>
    struct cdf
  {
    using value_type = bbm::iterable_value_t<C>;
    using index_type = index_t<value_type>;

    //! \brief Trivial constructor
    inline constexpr cdf(void) : _cdf() {}
    
    /********************************************************************/
    /*! \brief Constructor

      \param samples: a iterable container of samples (in order).

      The samples will be integrated into a CDF and normalized.
    *********************************************************************/
    inline constexpr cdf(const C& samples) : _cdf(samples)
    {
      if(bbm::size(samples) != 0)
      {
        std::partial_sum(bbm::begin(_cdf), bbm::end(_cdf), bbm::begin(_cdf));
        auto norm = *std::prev(bbm::end(_cdf));
        std::transform(bbm::begin(_cdf), bbm::end(_cdf), bbm::begin(_cdf), [&norm](auto in) { return in / norm; });
      }
    }

    //! \brief Copy Constructor
    inline constexpr cdf(const cdf& src) : _cdf(src._cdf) {}

    //! \brief Assigmemnt
    inline constexpr cdf operator=(const cdf& src)
    {
      if(&src == this) return *this;
      _cdf = src._cdf;
      return *this;
    }

    //! \brief number of samples
    inline size_t size(void) const { return _cdf.size(); }

    /********************************************************************/
    /*! \brief sample the CDF given a random variable xi

      \param xi = random variable between 0 and 1
      \param mask = enable/disable lanes.
      \returns sampled index, the pdf, and the residual
      
      The value type and index type are determined by the container itself.
      The random variable is expected to be provided in the same data type as
      the value type of the container.  Hence, if the container stores a
      packet type, the we also expect the random variable be given as a
      packet, and the index type to have the same number of channels.

      The returned type is a tuple of:
      + the sampled index based on xi
      + the corresponding pdf of the sample
      + the residual, i.e., the rescaled entropy of xi that was not used to determine the sample.
    *********************************************************************/
    inline auto sample(const value_type& xi, index_mask_t<index_type> mask=true) const
    {
      // make sure xi is in range
      mask &= bbm::cast<index_mask_t<index_type>>((xi >= 0) && (xi <= 1));

      // find index
      index_type idx = bbm::binary_search(_cdf, [&](const value_type& val) { return val < xi; }, mask);

      // compute pdf and residual
      mask &= (idx < bbm::size(_cdf));
      
      value_type eval = bbm::lookup<value_type>(_cdf, idx, mask);
      value_type prev = bbm::lookup<value_type>(_cdf, idx-1, mask && (idx >= 1));

      value_type pdf = eval - prev;
      value_type residual = bbm::select(mask, (xi - prev) / pdf, 0);
      
      // Done.
      return make_named<"index", "pdf", "residual">(idx, pdf, residual);
    }
    
    /********************************************************************/
    /*! \brief querry the PDF that an index will be sampled.
      
      \param idx = index to sample (must be in [0, size()-1]
      \param mask = enable/disable lanes
      \returns the pdf, which is equivalent to the normalized sample value.
    *********************************************************************/
    inline auto pdf(const index_type& idx, index_mask_t<index_type> mask=true) const
    {
      mask &= (idx < bbm::size(_cdf));

      // querry _cdf
      value_type eval = bbm::lookup<value_type>(_cdf, idx, mask);
      value_type prev = bbm::lookup<value_type>(_cdf, idx-1, mask && (idx >= 1));
      
      // compute pdf
      return (eval - prev);
    }

  private:
    //////////////////
    //! \brief Data
    //////////////////
    std::decay_t<C> _cdf;
  };

} // end bbm namespace

#endif /* _BBM_CDF_H_ */
