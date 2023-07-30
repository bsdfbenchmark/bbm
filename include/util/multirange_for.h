#ifndef _BBM_MULTI_RANGE_FOR_H_
#define _BBM_MULTI_RANGE_FOR_H_

#include <tuple>
#include <cassert>
#include <functional>

#include "core/error.h"
#include "concepts/util.h"
#include "util/iterator_util.h"

/***********************************************************************/
/*! \file multirange_for.h
    \brief ranged for loop over multiple containers at once
************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief ranged for loop over multiple containers at once

    \param func = function to call in each loop. The function is expected to
    return void, and takes as many arguments as there are containers. For each
    call, a reference to the corresponding element in each container is
    passed.
    \param containers... = containers. If not iterable, then the value is passed to each call of FUNC.
    
    Example:

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
    multirange_for( [&](auto& a_element, auto& b_element)
    {
       a += b;
    }, a_container, b_container);
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    This method will compute 'a_container += b_container'.

    Note: all containers must either have size==1 or the same size.
    
  ***********************************************************************/
  template<typename FUNC, typename... Ts>
    inline void multirange_for(FUNC&& func, Ts&&... containers)
  {
    // nothing to do
    if constexpr (sizeof...(Ts) == 0) return;
    
    // determine number of iterations
    size_t size = 0;
    ((size = std::max(size, bbm::size(containers))), ... );

    // validate the size of each container (must be 1 or 'size')
    if ( !((bbm::size(containers) == 1 || bbm::size(containers) == size) && ...) ) throw bbm_size_error;

    // create iterator for all containers (using bbm::begin instead of std::begin!)
    auto itr = std::make_tuple( (bbm::begin(containers))... );

    // helper lambdas
    auto idx = std::make_index_sequence<sizeof...(Ts)>{};
    auto incr = []<typename C>(C&& /*container*/, auto& itr) { if constexpr (std::ranges::range<C>) return ++itr; };
    auto incr_all = [&]<size_t... IDX>(std::index_sequence<IDX...>) { (incr(containers, std::get<IDX>(itr)), ...); };
    auto apply_all = [&]<size_t... IDX>(std::index_sequence<IDX...>) { std::invoke(std::forward<FUNC>(func), (*std::get<IDX>(itr))...); };
      
    // loop over all elements and call 'func'. Increment itr after every loop.
    for(size_t i=0; i != size; ++i, incr_all(idx))
      apply_all(idx);

    // Done.
  }

}

#endif /* _BBM_MULTI_RANGE_FOR_H_ */
