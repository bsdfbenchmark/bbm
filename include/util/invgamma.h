#ifndef _BBM_INVGAMMA_H_
#define _BBM_INVGAMMA_H_

#include "util/poly.h"
#include "util/gamma.h"

/************************************************************************/
/*! \file invgamma.h

  \brief Packet friendly computation of the the inverse of the incomplete
  gamma functions:
  + gamma_p_inv: inverse (x) of p = gamma_p(a, x)
  + gamma_q_inv: inverse (x) of q = gamma_q(a, x)
  
  Follows: "Computation of the Incomplete Gamma Function Ratios and their
  Inverse", DiDonata and Morris [1986]: https://doi.org/10.1145/22721.23109

*************************************************************************/

namespace bbm {
  namespace detail {
    namespace inverse_gamma {
      
      //! \brief Eq 21
      //! b > 0.6 || (b > 0.45 && a >= 0.3)
      template<typename T, typename Mask>
        inline T eq21(const T& a, const T& p, const T& q, const T& gamma, const T& b, Mask todo)
      {
        // exit if nothing to do
        if(bbm::none(todo)) return 0;
        
        // compute 'u'
        T u = 0;
        Mask mask = todo && ((b*q > 1e-8) && (q > 1e-5));
        todo &= !mask;
        if(bbm::any(mask)) u = bbm::select(mask, bbm::pow(p * gamma*a, 1/a), u);
        if(bbm::any(todo)) u = bbm::select(todo, bbm::exp((-q / a) - std::numbers::egamma), u);
       
        // Eq21
        return (u / (1 - (u / (a+1))));
      }
      
      //! \brief Eq 22
      //! a < 0.3 && 0.35 <= b <= 0.6
      template<typename T>
        inline T eq22(const T& /*a*/, const T& b)
      {
        T t = bbm::exp(-std::numbers::egamma - b);
        T u = t * bbm::exp(t);
        return t*bbm::exp(u);
      }
      
      //! \brief Eq 23
      //! (.15 <= b < 0.35) || (.15 <= b < 0.45 and a >= 0.3)
      template<typename T>
        inline T eq23(const T& a, const T& y)
      {
        T u = y - ((1-a) * bbm::log(y));
        return y - ((1-a) * bbm::log(u)) - bbm::log(1 + ((1-a)/(1+u)));
      }
      
      //! \brief Eq 24
      //! 0.01 < b < 0.15 
      template<typename T>
        inline T eq24(const T& a, const T& y)
       {
         T u = y - ((1-a) * bbm::log(y));
         return y - ((1-a) * bbm::log(u)) - bbm::log(bbm::poly(u, (2-a)*(3-a), 2*(3-a), 1) / bbm::poly(u, 2, (5-a), 1));
       }

      //! \brief Eq 25
      template<typename T>
        inline T eq25(const T& a, const T& y)
      {
        T a2 = a*a;
        T a3 = a2*a;
        
        T c1 = (a-1) * bbm::log(y);
        T c2 = (a-1) * (1+c1);
        T c3 = (a-1) * bbm::poly(c1, 0.5*(3*a-5), (a-2), -0.5);
        T c4 = (a-1) * bbm::poly(c1, (11*a2 - 46*a + 47) / 6.0, (a2 - 6*a + 7), -0.5*(3*a-5), 1.0/3.0);
        T c5 = (a-1) * bbm::poly(c1, (25*a3 - 195*a2 + 477*a - 379) / 12.0, (2*a3 - 25*a2 + 72*a - 61) * 0.5, (-3*a2 + 13*a - 13), (11*a - 17) / 6.0, -0.25);

        // done.
        return y + bbm::poly( bbm::rcp(y), c1, c2, c3, c4, c5);
      }

      //! \brief Eq 32: Q(a,x) = 1/2 erfc(s/sqrt(2)) => solve for s with minmax approximation
      template<typename T>
        inline T eq32(const T& p, const T& q)
      {
        T t = bbm::select(p < 0.5, bbm::sqrt(-2*bbm::log(p)), bbm::sqrt(-2*bbm::log(q)));
        T s = t - bbm::poly(t, 3.31125922108741, 11.6616720288968, 4.28342155967104, 0.213623493715853) / bbm::poly(t, 1, 6.61053765625462, 6.40691597760039, 1.27364489782223, 0.3611708101884203e-1);
        return bbm::select(p < 0.5, -s, s);
      }

      //! \brief Eq 31: Cornish-Fisher 6-term approximation for x
      template<typename T>
        inline T eq31(const T& a, const T& p, const T& q)
      {
        // Eq 31
        T sqrta = bbm::sqrt(a);
        T s = eq32(p,q);
        T w = bbm::poly(s,
                        a - 1.0/3.0 + 16/(810*a),                    // s^0 term
                        sqrta - 7/(36*sqrta) - 433/(38880*a*sqrta),  // s^1 term
                        1.0/3.0 - 7/(810*a),                         // s^2 term
                        1/(36*sqrta) + 256/(38880*a*sqrta),          // s^3 term
                        -3/(810*a),                                  // s^4 term
                        9/(38880*a*sqrta));                          // s^5 term

        // Done.
        return w;
      }
    
      //! \brief Eq 33
      //! a < 500 or |1-w/a| > 1e-6 and p > 0.5
      template<typename T>
        inline T eq33(const T& a, const T& y, const T& w)
      {
        T u = y + ((a-1) * bbm::log(w)) - bbm::log(1 + (1-a)/(1+w));
        return y + ((a-1) * bbm::log(u)) - bbm::log(1 + (1-a)/(1+u));
      }
      
      //! \brief Eq 34: Sn
      // S0 = 1
      // Sn = 1 + sum_{i=1}^N x^i / (PI_{j=1}^i (a+j))
      template<size_t N, typename T>
        inline T Sn(const T& x, const T& a, const T& tolerance=0)
      {
        T sum = 1;
        T partial = 1;
        auto mask = (partial > tolerance);

        // max N iterations or until tolerance is met
        for(size_t i=1; i <= N && bbm::any(mask); ++i)
        {
          partial *= bbm::select(mask, x / (a + i), 0);  // only update partial if tolerance not met.
          sum += partial;
          
          // converged?
          mask = (partial > tolerance);
        }
        
        // Done.
        return sum;
      }

      //! \brief Eq 34: Fn
      // Fn(x) = exp((v+x-lnSn(x))/a)
      template<size_t N, typename T>
        inline T Fn(const T& x, const T& a, const T& v, const T& tolerance=0)
      {
        return bbm::exp((v + x - bbm::log(Sn<N>(x,a,tolerance)))/a);
      }
      
      //! \brief Eq 35
      //! a > 500 && p < 0.5 && w < 0.15*(a+1)
      template<typename T>
        inline T eq35(const T& a, const T& p, const T& w)
      {
        T v = bbm::log(p) + bbm::lgamma(a+1);
        
        T u1 = Fn<0>(w, a, v);
        T u2 = Fn<1>(u1, a, v);
        T u3 = Fn<2>(u2, a, v);
        T z = Fn<3>(u3, a, v);

        // Done.
        return z;
      }
    
      //! \brief Eq 36
      //! 0.01*(a+1) < z <= 0.7*(a+1)
      template<typename T>
        inline T eq36(const T& a, const T&p, const T& z)
      {
        T lnSn = bbm::log( Sn<100>(z, a, T(1e-4)) );
        T v = bbm::log(p) + bbm::lgamma(a + 1);
        T zbar = bbm::exp((v + z - lnSn) / a);
        T result = zbar * (1 - (a * bbm::log(zbar) - z - v + lnSn) / (a - zbar));
        
        // Done.
        return result;
      }


      //! \brief Eq 3
      //! R(a,x) for 20 > a > 0
      template<typename T>
        inline T eq3(const T& a, const T& x, const T& lg)
      {
        return bbm::exp(-x - lg + bbm::log(x)*a);
      }

      //! \brief Eq 4
      //! R(a,x) for a >= 20
      template<typename T>
        inline T eq4(const T& a, const T& x, const T& lg)
      {
        T lambda = x/a;
        T delta = lg - ((a - 0.5)*bbm::log(a)) + a - 0.5*bbm::log(2.0*std::numbers::pi);
        T phi = lambda - 1 - bbm::log(lambda);
        return bbm::sqrt(0.5*a/std::numbers::pi) * bbm::exp(-a*phi - delta);
      }

      //! \brief R(a,x)
      template<typename T>
        inline T R(const T& a, const T& x, const T& lg)
      {
        return bbm::select(a < 20, eq3(a,x,lg), eq4(a,x,lg));
      }
      
      /******************************************************************/
  
      //! \brief handle case where a < 1
      template<typename T, typename Mask>
        inline T a_less_one(const T& a, const T& p, const T& q, Mask todo, Mask& /*converged*/)
      {
        // exit if nothing to do
        if(bbm::none(todo)) return 0;

        // Handle 5 different cases
        T gamma = bbm::tgamma(a);
        T b = q * gamma;
        
        // Case 1: b > 0.6 || (b >= 0.45 && a >= 0.3)
        Mask mask = todo && ((b > 0.6) || (b >= 0.45 && a >= 0.3));
        todo &= !mask;
        T result = bbm::select(mask, eq21(a,p,q,gamma,b,mask), 0);
        if(bbm::none(todo)) return result;
      
        // Case 2: a < 0.3 && b >= 0.35
        mask = todo && ((a < 0.3) && (b >= 0.35));
        todo &= !mask;
        if(bbm::any(mask)) result = bbm::select(mask, eq22(a,b), result);
        if(bbm::none(todo)) return result;
        
        // Case 3: b >= 0.15 || a >= 0.3
        T y = -bbm::log(b);
        mask = todo && ((b >= 0.15) || (a >= 0.3));
        todo &= !mask;
        if(bbm::any(mask)) result = bbm::select(mask, eq23(a,y), result);
        if(bbm::none(todo)) return result;
      
        // Case 4: b > 0.1
        mask = todo && (b > 0.1);
        todo &= !mask;
        if(bbm::any(mask)) result = bbm::select(mask, eq24(a,y), result);
        if(bbm::none(todo)) return result;
      
        // Case 5: otherwise (b <= 0.1)
        result = bbm::select(todo, eq25(a,y), result);
        
        // Done.
        return result;
      }

      //! \brief handle case where a > 1 & p > 0.5
      template<typename T, typename Mask>
        inline T p_greater_half(const T& a, const T& q, const T& w, Mask todo, Mask& /*converged*/)
      {
        // exit if nothing to do
        if(bbm::none(todo)) return 0;

        // Case 1: w < 3*a
        Mask mask = todo && (w < 3*a);
        todo &= !mask;
        T result = bbm::select(mask, w, 0);
        if(bbm::none(todo)) return result;

        // Case 2: lb <= -D*2.3
        T D = bbm::max(a*(a-1), 2);
        T lg = bbm::lgamma(a);
        T lb = bbm::log(q) + lg;

        mask = todo && (lb <= -D*2.3);
        todo &= !mask;
        if(bbm::any(mask)) result = bbm::select(mask, eq25(a,-lb), result);
        if(bbm::none(todo)) return result;

        // Case 3: otherwise
        result = bbm::select(todo, eq33(a,-lb,w), result);

        // Done.
        return result;
      }
      
      //! \brief handle case where a > 1 & p < 0.5
      template<typename T, typename Mask>
        inline T p_less_half(const T& a, const T& p, const T& w, Mask todo, Mask& converged)
      {
        // exit if nothing to do
        if(bbm::none(todo)) return 0;

        // Compute z
        Mask mask = todo && (w < 0.15*(a+1));
        T z = w;
        if(bbm::any(mask)) z = bbm::select(mask, eq35(a,p,w), z);
        
        // Case 1: z < (0.01*(a+1) || z > 0.7*(a+1)
        mask = todo && (z < 0.01*(a+1) || z > 0.7*(a+1));
        todo &= !mask;
        converged |= (z <= 0.002*(a+1));
        T result = bbm::select(mask, z, 0);    // converged if z <= 0.002*(a+1)
        if(bbm::none(todo)) return result;

        // Case 2: otherwise
        result = bbm::select(todo, eq36(a,p,z), result);
        
        // Done.
        return result;
      }

      
      //! \brief handle case where a > 1
      template<typename T, typename Mask>
        inline T a_greater_one(const T& a, const T& p, const T& q, Mask todo, Mask& converged)
      {
        // exit if nothing to do
        if(bbm::none(todo)) return 0;

        // Eq 31
        T w = eq31(a,p,q);
        
        // Case 1: a >= 500 && |1-w/a| < 1e-6
        Mask mask = todo && (a >= 500 && bbm::abs(1-w/a) < 1e-6);
        todo &= !mask;
        converged |= mask;
        T result = bbm::select(mask, w, 0);   // Converged
        if(bbm::none(todo)) return result;
      
        // Case 2: p > 0.5
        mask = todo && (p > 0.5);
        todo &= !mask;
        result = bbm::select(mask, p_greater_half(a,q,w,mask,converged), result);
        if(bbm::none(todo)) return result;
        
        // Case 3: otherwise (p < 0.5)
        result = bbm::select(todo, p_less_half(a,p,w,todo,converged), result);
        
        // Done.
        return result;
      }

      /******************************************************************/

      /******************************************************************/
      /*! \brief Provides an initial estimate of the inverse (x) of (p,q) given an 'a'.

        \param a = 'a' parameter of the incomplete gamma functions
        \param p = upper normalized incomplete gamma function
        \param q = 1 - p (lower gamma)
        \param mask = enable/disbale lanes
        \param converged = will be set to true if the corresponding lanes in 'x' are converged to 10 digits.
        \returns the inverse: 'x' (also changes 'converged').
      *******************************************************************/
      template<typename T, typename Mask>
        inline T estimate(const T& a, const T& p, const T& q, Mask mask, Mask& converged)
      {
        T result = 0;

        // Case 1: a == 1
        auto mask1 = mask && (a == 1);
        converged |= mask1;
        if(bbm::any(mask1)) result = bbm::select(mask1, -bbm::log(q), result);
        if(bbm::all(mask1)) return result;

        // Case 2: a < 1
        auto mask2 = mask && (a < 1);
        result = bbm::select(mask2, a_less_one(a, p, q, mask2, converged), result);
        if(bbm::all(mask1 || mask2)) return result;
      
        // Case 3:
        auto mask3 = mask && (a > 1);
        result = bbm::select(mask3, a_greater_one(a, p, q, mask3, converged), result);
        
        // Done.
        return result;
      }

      /******************************************************************/
      /*! \brief compute (x) the inverse of (p,q) given an 'a'

        \tparam NUM_ITR = maximum number of iterations for Newtan-Raphson; 3 suffices
        \param a = 'a' parameter of the incomplete gamma function
        \param p = result of the upper normalized incomplete gamma function
        \param q = 1-p
        \param mask = enable/disable lanes
        \returns the inverse: 'x'
      *******************************************************************/
      template<typename T, typename Mask, size_t NUM_ITR=3>
        inline T inverse(const T& a, const T& p, const T& q, Mask mask)
      {
        // exit if nothing to do
        if(bbm::none(mask)) return 0;
        
        // get initial esitimate
        Mask converged = false;
        auto x = estimate(a, p, q, mask, converged);
        T lg = bbm::lgamma(a);
        
        // Newton-Raphson iterations
        for(size_t itr=0; itr < NUM_ITR && !bbm::all(converged); ++itr)
        {
          T r = R(a,x,lg);
          auto pq = bbm::gamma_pq(a,x);
          T t = bbm::select(p <= 0.5, (bbm::get<"p">(pq) - p), (q - bbm::get<"q">(pq))) / r;
          T w = 0.5 * (a - 1 - x);

          // update x
          auto m = (bbm::abs(t) <= 0.1) && (bbm::abs(w*t) <= 0.1);
          x *= 1 - bbm::select(!converged, t + bbm::select(m, w*t*t, 0), 0);
        }
        
        // Done.
        return x;
      }
      
    } // end inverse_gamme namespace
  } // end detail namespace


  /**********************************************************************/
  /*! \brief the inverse of the normalized upper incomplete gamma function

    \f$ x = P^{-1}(a, p)\f$

    such that \f$ p = P(a, x) \f$.
  ***********************************************************************/
  template<typename TA, typename TP>
    inline auto gamma_p_inv(const TA& a, const TP& p)
  {
    using result_t = decltype(a + p);
    auto mask = (a > 0) && (p > 0);
    return bbm::detail::inverse_gamma::inverse(result_t(a), result_t(p), result_t(1-p), mask);
  }

  /**********************************************************************/
  /*! \brief the inverse of the normalized lower incomplete gamma function

    \f$ x = Q^{-1}(a, q)\f$

    such that \f$ q = Q(a, x) \f$.
  ***********************************************************************/
  template<typename TA, typename TQ>
    inline auto gamma_q_inv(const TA& a, const TQ& q)
  {
    using result_t = decltype(a + q);
    auto mask = (a > 0) && (q > 0);
    return bbm::detail::inverse_gamma::inverse(result_t(a), result_t(1-q), result_t(q), mask);
  }

  
} // end bbm namespace


#endif /* _BBM_INVGAMMA_H_ */
