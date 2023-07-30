/************************************************************************/
/*! \file he_westin.cpp

  \brief Compare BBM's he_westin BSDF model to the reference implementation by
  Steve Westin: http://www.graphics.cornell.edu/~westin/heBRDF/heBRDF.tar.gz
  with the following bugs fixed in the Vector3 class: z component in 'length'
  and 'MakeUnitVector', the z component of 'other' in operator-, operator+,
  and the y-component in the solution of the cross product.  Note: Westin et
  al.'s code has memory leaks; we did not fix these.

*************************************************************************/
  
#include "bbm.h"
using namespace bbm;
BBM_IMPORT_CONFIG(floatRGB)


/************************************************************************/
/*! Westin et al.'s implementation below
*************************************************************************/
#include <cstdio>
#include <iostream>

#ifndef M_PI
#define M_PI                3.14159265358979323846
#endif // M_PI

static const float ANGLE_EPS = 1.0e-5;	     // Angle less than this is zero
static const float STINY = 1.0e-4; // in micro
static const float tinyEpsilon = 1.0e-12;
static const float smallEpsilon = 1.0e-8;

class Vector3 {

 private:
  float _tol;

 public:
  float _val[3];


  Vector3() {}
  Vector3 ( float x, float y, float z ) {
    this->set ( x, y, z );
  }

  inline void set ( float x, float y, float z ) {
    _val[0] = x;
    _val[1] = y;
    _val[2] = z;
  }

  inline float length() const {
    return sqrt ( _val[0]*_val[0] + _val[1]*_val[1] + _val[2]*_val[2] );
  }

  inline void MakeUnitVector() {
    float len = _val[0]*_val[0] + _val[1]*_val[1] + _val[2]*_val[2];

    if ( fabs ( len ) < _tol )
      return;
    else {
      len = sqrt ( len );
      this->_val[0] /= len;
      this->_val[1] /= len;
      this->_val[2] /= len;
    }
  }

  inline Vector3 operator-() const { return Vector3(-_val[0],-_val[1],-_val[2]); }

  inline int operator==(const Vector3 other) const {
    return ( _val[0] == other[0] &&  _val[1] == other[1] && _val[2] == other[2] );
  }

  inline Vector3 operator*(const float scale) const {
    return Vector3(scale*_val[0],scale*_val[1],scale*_val[2]);
  }

  inline Vector3 operator-(const Vector3 other) const {
    return Vector3(_val[0]-other[0],_val[1]-other[1],_val[2]-other[2]);
  }

  inline Vector3 operator+(const Vector3 other) const {
    return Vector3(_val[0]+other[0],_val[1]+other[1],_val[2]+other[2]);
  }

  inline float operator[] ( const int i ) const {
    return _val[i];
  }

  inline float& operator[] ( const int i ) {
    return _val[i];
  }

};

Vector3 operator*(const float scale, Vector3 v) {
  return v*scale;
}

Vector3 cross ( const Vector3 a, const Vector3 b ) {
  Vector3 result;

  (result)[0] =   a._val[1]*b._val[2] - a._val[2]*b._val[1];
  (result)[1] =   a._val[2]*b._val[0] - a._val[0]*b._val[2];
  (result)[2] =   a._val[0]*b._val[1] - a._val[1]*b._val[0];
  return result;
}

inline float dot ( const Vector3 a, const Vector3 b ) {
  return ( a._val[0]*b._val[0] + a._val[1]*b._val[1] + a._val[2]*b._val[2] );
}


inline float SQR(const float& x) {
  return (x*x);
}

// Return an arbitrary unit vector perpendicular to v:
Vector3 PerpUnitVector3(const Vector3 &v) {

  Vector3 u = Vector3(1,0,0);
  if ( dot(u,v)<tinyEpsilon )
    u.set(0,1,0);
  Vector3 w = cross(v,u);
  w.MakeUnitVector();
  return w;
}

class heBRDF {
public:

  // constructor:
  heBRDF(const float sigma0_, const float tau_, // in micro
         const float *lambda_, int nLambda_,     // in nanonmeter:
         const float *k1_, const float *k2_,  // index of refraction/extinction
         const float *rho_ud_=0);

  // destructor:
  ~heBRDF();

  //----------------------------------------------------
  // BRDF computation:
  //
  // (1) I try to group the computation for spectral terms separately from
  // angular terms as much as possible to avoid redundant calculation.
  //
  // (2) I try to use dimensionless quantities as much as possible to
  // increase the precision in the computation:
  //

  // directional diffuse BRDF: Equ(71)
  // return a spectrum in rho:
  float *rho_dd (const Vector3& w_i, const Vector3& w_r, const Vector3& n);

  // specular component: Equ(70)
  float *rho_sp (const Vector3& w_i, const Vector3& w_r, const Vector3& n,
                  const float& dw_i);

  // total reflectance: Equ(69)
  float *rho_tot (const Vector3& w_i, const Vector3& w_r, const Vector3& n,
                   const float& dw_i);

  // specular coefficients:   Equ(73, 74)
  // Only used when the incident direction is within the solid angle Delta
  // return a spectrum in rho
  float *rho_s (const Vector3& w_i, const Vector3& n);

  //------------------------------------------------------
  // Functions depending only on angular configuration:

  float S_func(const float& cot_theta) const; // Eq. 24

  float Sigma_func (const float &cos_theta_i,    // Eq. 80
                     const float &cos_theta_r) const ;

  float K_func(const float& cot_theta) const;    // Eq. 82, 83

  float G_func(const Vector3& w_i,          // Eq. 76
                const Vector3& w_r,
                const Vector3& n) const;

  //------------------------------------------------
  // Functions depending both on angles and on wavelength:

  float FresnelCoef(const float& cos_theta_i, // Eq. 75
                     const float& k1_,
                     const float& k2_) const ;

  float D_func(const float& g,             // Eq. 78
                const float& tau_over_lambda,
                const float& v_xy2) const;

  //------------------------------------------------
  // Data members:

  // RMS roughness and autocorrelation length
  float sigma0, tau;

  // Wavelength, real and imaginary parts of refractive index
  float *lambda, *k1, *k2;

  // Number of elements in arrays lambda, k1, k2
  int nLambda;

  // Uniform diffuse (Lambertian) term
  float *rho_ud;
};

// constructor:
heBRDF::heBRDF(const float sigma0_, const float tau_, // in micrometers
               const float *lambda_, int nLambda_, // in nanometers:
               const float *k1_, const float *k2_, // index of
					     // refraction/extinction
               const float *rho_ud_) {

  int i;

  sigma0 = sigma0_ ;
  tau = tau_ ;

  nLambda = nLambda_;
  lambda = new float [nLambda];
  k1 = new float [nLambda];
  k2 = new float [nLambda];
  rho_ud = new float [nLambda];

  for (i=0;i<nLambda;i++) {
    lambda[i] = lambda_[i] * 0.001;  // conversion from nanometer to micron
    k1[i] = k1_[i];
    k2[i] = k2_[i];
  }

  if (rho_ud_) {
    float dLambda = lambda[1] - lambda[0];
    float sc = 1.0/M_PI;
    for (int i=0;i<nLambda;i++) {
      rho_ud[i] = sc*rho_ud_[i];    // scale to the proper unit:
    }
  }
  else {
    for (int i=0;i<nLambda;i++) {
      rho_ud[i] = 0;
    }
  }
}

// destructor:
heBRDF::~heBRDF() {
  delete lambda;
  delete k1;
  delete k2;
  delete rho_ud;
}

// specular component: Equ(70)
float* heBRDF::rho_sp (const Vector3& w_i, const Vector3& w_r,
                        const Vector3& n, const float& dw_i) {

  float *Rho = new float [nLambda];

  Vector3 w_mirror = 2*dot(w_i, n) * n - w_i;

  if ( dot(w_mirror, w_r)>= cos(sqrt(dw_i/M_PI)) ) {

    float *Rho_s = rho_s(w_i, n);
    for (int i=0;i<nLambda;i++) {
      Rho[i] = Rho_s[i] / ( dot(w_i, n) * dw_i) ;
    }
    delete [] Rho_s;
  }
  else {
    for (int i=0;i<nLambda;i++) {
      Rho[i] = 0;
    }
  }
  return (Rho);
}

// total reflectance: Equ(69)
float* heBRDF::rho_tot(const Vector3& w_i, const Vector3& w_r,
                        const Vector3& n, const float& dw_i) {

  float *Rho_dd = rho_dd (w_i, w_r, n);
  float *Rho_sp = rho_sp (w_i, w_r, n, dw_i);

  float *Rho = new float [nLambda];
  for (int i=0;i<nLambda;i++) {
    Rho[i] = Rho_dd[i] + Rho_sp[i] + rho_ud[i];
  }

  delete [] Rho_dd;
  delete [] Rho_sp;

  return (Rho);
}

// specular coefcients:   Equ(73, 74)
float* heBRDF::rho_s (const Vector3& w_i, const Vector3& n) {

  float cos_theta_i = dot(w_i, n);
  float sin_theta_i = sqrt ( 1.0 - SQR(cos_theta_i) );
  float cot_theta_i = cos_theta_i / sin_theta_i;

  //--------------------------------------------------
  // spatial-only dependent terms:
  float S;
  if ( sin_theta_i > ANGLE_EPS )	     // SHW 04/08/98: Provide for
    S =	 SQR(S_func(cot_theta_i));	     // theta_i -> 0
  else
    S = 1.0;

  // sigma: Equ(80)

  float sigma;
  /*
  if ( sigma0 < STINY )
    sigma = sigma0;
  else
  */
  sigma = Sigma_func(cos_theta_i, cos_theta_i);

  float *rho = new float [nLambda];

  //----------------------------------------------------
  //Spectral-Spatial coupled terms:
  for (int i=0;i<nLambda;i++) { // spectral sampling:
    float g = SQR(2.0*M_PI*(sigma/lambda[i])*(cos_theta_i+cos_theta_i));
    float F = FresnelCoef(cos_theta_i, k1[i], k2[i]);
    rho[i] = S*F*exp(-g);
  }
  return (rho);
}

//---------------------------------------------------------
// Functions depending only on angular configuration:
//--------------------------------------------------------

// Shadowing function: Equ(24)
float
heBRDF::S_func(const float& cot_theta) const {

  // float tau_over_sigma = tau/sigma0;
  if ( sigma0 < STINY ) return 1.0;	     // SHW 04/08/98 Allow smooth
					     // surface
  float cotexp = cot_theta*tau/(sigma0*2);
  float erfy = erfc(cotexp);

  float capitalLambda = 0.5 *
    ( exp(-SQR(cotexp))/(sqrt(M_PI)*cotexp) - erfy );

  float S = (1.0 - 0.5*erfy) / (capitalLambda + 1.0);
  return S;
}


// Get Fresnel Coef from Index of refraction/extinction
//  |F|^2  : Equ(75)
float
heBRDF::FresnelCoef(const float& cos_theta_i, const float& k1_, const float& k2_) const {

  float c = cos_theta_i;
  float s = sqrt(1-SQR(c));
  float c2 = SQR(c);
  float s2 = SQR(s);
  float t = s/c;
  float st = s*t;
  float st2 = SQR(st);

  float B = SQR(k1_) - SQR(k2_)-s2;
  float A = sqrt(SQR(B)+4*SQR(k1_*k2_));

  float  a2 = 0.5*(A+B);
  float  b2 = 0.5*(A-B);
  float  a = sqrt(a2);

  float  Fs = (a2+b2-2*a*c+c2)/(a2+b2+2*a*c+c2);
  float  Fp = Fs * (a2+b2-2*a*st+st2)/(a2+b2+2*a*st+st2);

  float F = 0.5*(Fs+Fp);

  return (F);

}


// Routine to find effective surface roughness
// involves Newton-Raphson method for finding z0
// Equ (80)
#define MAXITER 100
#define ACCURACY 1e-8
#define f_ACCURACY 1e-8
float
heBRDF::Sigma_func (const float &cos_theta_i,
                    const float &cos_theta_r) const {

  if ( fabs(sigma0) < ACCURACY ) return 0.0;
  if ( cos_theta_i < ACCURACY || cos_theta_r < ACCURACY )
    return 0.0;				     // Avoid divide-by-zero
  // get K_i, K_r:
  float tan_theta_i = sqrt(1-SQR(cos_theta_i))/ cos_theta_i;
  float tan_theta_r = sqrt(1-SQR(cos_theta_r))/ cos_theta_r;

  float K = K_func(tan_theta_i) + K_func(tan_theta_r);

  if (K==0)   // trivial case
    return sigma0;

  // calculate z0/sigma0 <- x
  float x, expn, func, func_prev, df_dx, dx;
  float f0 = sqrt(1.0/(8.0*M_PI)) * K;
  
  // A carefully choosen initial value,
  // convergent with 15 iteration per solving when f0=0->10^5:
  if (f0<=1)
    x = f0;
  else
    x = sqrt(2*log(f0));

  func_prev = -1;
  int i;
  for (i=0; i<MAXITER; i++) {

    // Evaluate the function and its derivative
    expn = exp(0.5*x*x);
    func = x * expn - f0;
    df_dx = (1+x*x)*expn;

    // predict next position with Newton method:
    dx = func / df_dx;
    x -= dx;

    if ( (fabs(dx) < ACCURACY) && ( fabs(func-func_prev)<f_ACCURACY) )
      break;

    func_prev = func;
  }

  float eff_sigma = sigma0 / sqrt(1 + SQR(x));
  return eff_sigma;
}


// Equ: (82-83)
float heBRDF::K_func(const float& tan_theta ) const {

  // float eps = 1.0e-4;  // 20 arc second
  float eps = 1.0e-6;
  float maxNum = 1.0e+12;  // overflow:
  float K;

  // if (0.5*tau/sigma0*cot_theta)>5, then K<tan_theta*E-12
  if (fabs(tan_theta)< eps)
    K = 0;
  else if (tan_theta< maxNum)
    K = tan_theta * erfc(0.5*tau/(sigma0*tan_theta));
  else
    K =  maxNum;

  return K;
}


// Geometry factor G: Equ(76)
float heBRDF::G_func(const Vector3& w_i,
                      const Vector3& w_r,
                      const Vector3& n) const {

  Vector3 k_i = -w_i;
  Vector3 k_r = w_r;

  if (w_i == w_r) {  // in case same direction:
    k_r = k_r + smallEpsilon*Vector3(1,1,1);
    k_r.MakeUnitVector();
  }

  Vector3 s_i = cross(k_i, n);
  if (s_i.length() > smallEpsilon)
    s_i.MakeUnitVector();
  else
    s_i = PerpUnitVector3(n);

  Vector3 s_r = cross(k_r, n);
  if (s_r.length() > smallEpsilon)
    s_r.MakeUnitVector();
  else
    s_r = PerpUnitVector3(n);

  // sure perpendicular:
  Vector3 p_i = cross(s_i, k_i);
  Vector3 p_r = cross(s_r, k_r);

  Vector3 v = k_r - k_i;
  float v_term = SQR(dot(v,v)/dot(v, n));

  Vector3 crossVec = cross(k_i, k_r);

  float dot_term = ( SQR(dot(s_r, k_i)) + SQR(dot(p_r, k_i)) )
    * ( SQR(dot(s_i, k_r)) + SQR(dot(p_i, k_r)) );

  float G = v_term * dot_term / SQR(dot(crossVec, crossVec));
  
  return (G);
}

// last term of D summation
#define Dsumlim 1.0e-12
// maximum terms in D summation; this avoids overflow of m! term */
#define Dsummax 1000000

// Eq. 78
float heBRDF::D_func(const float& g,
                      const float& tau_over_lambda,
                      const float& v_xy2) const {

  float D;

  if ( g > 18.5 ) {
    // Rough surface: use the approximation from Beckmann, 1963 edition,
    // section 5.3, equation 47, page 87.
    D = SQR(M_PI/2.0*tau_over_lambda) / g
      * exp ( -v_xy2 *SQR(2*M_PI*tau_over_lambda) / (4.0*g) );
  }
  else {
    // Compute as the sum of an infinite series. (Section 5.3, Equation 35,
    // Page 86 of Beckmann '63.)
    // The series is a little odd, in that magnitude of the terms tends
    // to increase, then decrease. We terminate when terms get really small
    // AND are diminishing.

    float v_xy_tau_2 = v_xy2 * SQR(tau_over_lambda*2*M_PI);

    float sum = 0.0;
    float m_fact = 1.0;
    float this_term = 0.0,
           last_term = -1.0;

    for ( int m=1;
	  ( this_term > Dsumlim ) || ( this_term > last_term );
	  m++ ) {
      
      // Multiply the factor by g/m to update the g**m/(m!) term. */
      m_fact *= g / (float)m;
      last_term = this_term;
      // exp(-709) is about the point of underflow in float precision
      // (result is 1e-308). If the argument to the exponential is more
      // negative than this, the answer will always be zero, and we can
      // avoid the function call.
      float exp_base = -g - v_xy_tau_2/(4.0*m);
      if ( exp_base > -750.0 ) {
	this_term = exp ( exp_base ) * m_fact / m;
	sum += this_term;
      }
      else
	this_term = 0.0;

    }
    
    D =  sum * SQR(0.5*M_PI*tau_over_lambda);

  }

  return (D);

}


// directional diffuse BRDF: Equ(71)
float* heBRDF::rho_dd (const Vector3& w_i, const Vector3& w_r,
                        const Vector3& n) {

  float cos_theta_i = dot(w_i, n);
  float sin_theta_i = sqrt ( 1.0 - SQR(cos_theta_i) );
  float cot_theta_i = cos_theta_i / sin_theta_i;

  float cos_theta_r = dot(w_r, n);
  float sin_theta_r = sqrt ( 1.0 - SQR(cos_theta_r) );
  float cot_theta_r = cos_theta_r / sin_theta_r;

  // Compute half the angle between incident and exitant directions,
  // by means of the trigonometric identity
  // cos(x/2) = +- sqrt((1+cos(x))/2).
  // Since we know that 0 <= x <= pi, we can neglect the negative
  // branch.
  float cos_half_angle = sqrt ( (1+dot(w_i,w_r)) / 2 );

  //--------------------------------------------------
  // spatial dependent terms:
  // spatial-only dependent terms:
  float S = 1.0;
  if ( sin_theta_i > ANGLE_EPS )	     // 04/08/98 SHW: Provide for
    S *= S_func(cot_theta_i);		     // theta -> 0
  if ( sin_theta_r > ANGLE_EPS )
    S *= S_func(cot_theta_r);

  // sigma: Equ(80)
  float sigma;

  /*
  if ( sigma0 < STINY )
    sigma = sigma0;
  else
  */
  sigma = Sigma_func(cos_theta_i, cos_theta_r);

  // G:
  float G = G_func(w_i, w_r, n);

  // spatial term:
  float spatial_term;
  if ( (G*S) ==0)
    spatial_term = 0;
  else
    spatial_term = G*S/(M_PI*cos_theta_i*cos_theta_r);

  // v_xy:
  Vector3 v = w_r + w_i;   // k_r - k_i
  float v_xy2 = dot(v,v) - SQR(dot(v, n));

 //----------------------------------------------------
  //Spectral-Spatial coupled terms:
  float g, F, D;

  float *rho = new float [nLambda];
  for (int i=0;i<nLambda;i++) { // spectral sampling:
    g = SQR(2*M_PI*(sigma/lambda[i])*(cos_theta_i+cos_theta_r));
    F = FresnelCoef(cos_half_angle, k1[i], k2[i]);
    D = D_func(g, (tau/lambda[i]), v_xy2);
    rho[i] = spatial_term * F * D;
  }
  
  return (rho);
}

/************************************************************************/

int main(int argc, char** argv)
{
  // Number of in-out directions to try
  size_t maxTests = 16;

  // Using some reasonable set of parameters for testing
  Spectrum r(1.3211, 0.88045, 0.59765);
  Spectrum i(7.41785, 6.1455, 5.2337);
  ior::complex_ior<Spectrum> ior(r, i);
  Value roughness = 0.0318;
  Value autocorrelation = 0.3;
  Spectrum w(645, 526, 444);
  
  
  heBRDF ref(roughness, autocorrelation,
             w.begin(), 3,
             r.begin(), i.begin());
             
  hewestin<Config> h(roughness, autocorrelation, ior);

  for(size_t idx=0; idx < maxTests; ++idx)
  {
    Vec3d in = bbm::normalize(Vec3d(drand48(), drand48(), drand48()));
    Vec3d out = bbm::normalize(Vec3d(drand48(), drand48(), drand48()));
    Vector3 i(in[0], in[1], in[2]);
    Vector3 o(out[0], out[1], out[2]);
    Vector3 n(0,0,1);
    
    std::cerr << in << " -> " << out << " : ";

    float* c = ref.rho_dd(i, o, n);
    std::cerr << c[0] << ", " << c[1] << "," << c[2]<< " vs ";
    auto v = h.eval(in, out);
    std::cerr << v;
    std::cerr << " (diff = " << c[0] - v[0] << ", " << c[1] - v[1] << ", " << c[2] - v[2] << ")";
    std::cerr << std::endl;
    delete[] c;
  }


  // Done.
  return 0;
}
