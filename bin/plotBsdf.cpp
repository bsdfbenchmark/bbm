#include <iostream>
#include <random>

#include "bbm.h"
#include "core/bitmap.h"
#include "util/option.h"
#include "io/pfm.h"

#ifdef BBM_PYTHON
  #include "python/bbm_python_interpreter.h"
#elif !defined(BBM_BSDF)
  #define BBM_BSDF lambertian<floatRGB>()
#endif /* BBM_PYTHON */

using namespace bbm;

BBM_IMPORT_CONFIG( floatRGB );


bitmap<Spectrum> evaluate(const BsdfPtr& bsdf, size_t width, size_t height, const Vec3d& view, size_t samples)
{
  std::mt19937 rnd;
  std::uniform_real_distribution<float> U(0,1);
  bitmap<Spectrum> result(width, height);
  Vec2d sph_coord;
  
  for(size_t y=0; y < height; ++y)
    for(size_t x=0; x < width; ++x)
      for(size_t s=0; s < samples; ++s)
      {
        spherical::phi(sph_coord) = Constants::Pi(2) * (x + U(rnd)) / width;
        spherical::theta(sph_coord) = Constants::Pi() * (y + U(rnd)) / height;        
        Vec3d light = spherical::convert( sph_coord );

        result(x,y) += bsdf->eval(light, view) * spherical::cosTheta(light) / samples;
      }

  // Done.
  return result;
}


bitmap<Spectrum> pdf(const BsdfPtr& bsdf, size_t width, size_t height, const Vec3d& view, size_t samples)
{
  std::mt19937 rnd;
  std::uniform_real_distribution<float> U(0,1);
  bitmap<Spectrum> result(width, height);
  Vec2d sph_coord;

  for(size_t y=0; y < height; ++y)
    for(size_t x=0; x < width; ++x)
    {
      for(size_t s=0; s < samples; ++s)
      {
        spherical::phi(sph_coord) = Constants::Pi(2) * (x + U(rnd)) / width;
        spherical::theta(sph_coord) = Constants::Pi() * (y + U(rnd)) / height;        
        Vec3d light = spherical::convert( sph_coord );
        Value pdf = Constants::Pi2(2) * bbm::abs(spherical::sinTheta(sph_coord)) / (width * height);
        
        result(x,y) += bsdf->pdf(light, view) * pdf;
      }
      result(x,y) /= samples;
    }

  // Done.
  return result;
}


bitmap<Spectrum> sample(const BsdfPtr& bsdf, size_t width, size_t height, const Vec3d& view, size_t samples, bool maskZero)
{
  std::mt19937 rnd;
  std::uniform_real_distribution<float> U(0,1);
  bitmap<Spectrum> result(width, height);
  
  for(size_t s=0; s < samples*width*height; ++s)
  {
    auto sample = bsdf->sample(view, Vec2d(U(rnd), U(rnd)));
    Vec2d sph_coord = spherical::convert(sample.direction);

    size_t x = bbm::min( spherical::phi(sph_coord) / Constants::Pi(2) * width, width-1);
    size_t y = bbm::min( spherical::theta(sph_coord) / Constants::Pi() * height, height-1);

    if(!maskZero || sample.pdf > Constants::Epsilon())
      result(x,y) += 1.0 / (float)(width*height*samples);
      
  }

  // Done.
  return result;
}


int main(int argc, char** argv)
{
  ////////////////
  // Parse Input
  ////////////////
  if(argc == 1)
  {
    std::cout << "Usage: " << argv[0] << " [bsdfmodel=<python string>] [filename=<name>] [width=512] [height=256] [view=[0,0,1]] [samples=1] [scale=1] [maskZero] [plot=<eval|pdf|sample>]" << std::endl;
    #ifndef BBM_PYTHON
      std::cout << "BBM compiled without Python support. <bsdfmodel> is ignored and " << BBM_STRINGIFY(BBM_BSDF) << " is used instead." << std::endl;
    #endif
    return -1;
  }

  auto opt = parse_options(argc, argv);

  auto bsdfmodel = get<string_option>(opt, "bsdfmodel");
  auto filename = get<string_option>(opt, "filename");
  auto width = get<int_option>(opt, "width", 512);
  auto height = get<int_option>(opt, "height", 256);
  auto samples = get<int_option>(opt, "samples", 1);
  auto view = get<vec3d_option<float_option>>(opt, "view", Vec3d(0,0,1));
  auto plot = get<string_option>(opt, "plot");
  auto scale = get<float_option>(opt, "scale", 1.0);
  bool maskZero = get<bool_option>(opt, "maskZero", false);
  
  /////////////////////////
  // Validate options
  ////////////////////////
  if(plot != "eval" && plot != "pdf" && plot != "sample")
  {
    std::cout << "ERROR: unknown plotting command '" << plot << "'." << std::endl;
    return -1;
  }

  if(filename == "")
  {
    std::cout << "ERROR: expected an output fileame." << std::endl;
    return -1;
  }
  
  ////////////////
  // Create BSDF
  ////////////////
  #ifdef BBM_PYTHON
    if(bsdfmodel == "")
    {
      std::cout << "ERROR: bsdfmodel not specified." << std::endl;
      return -1;
    }
     BsdfPtr bsdf = python::capture<BsdfPtr>(bsdfmodel);
  #else
    if(bsdfmodel != "")
      std::cout << "Ignoring user specified BSDF (" << bsdfmodel << "), and using BBM_BSDF=" << BBM_STRINGIFY(BBM_BSDF) << " instead." << std::endl;
    BsdfPtr bsdf = make_bsdf_ptr(BBM_BSDF);
  #endif /* BBM_PYTHON */

  //////////////////////
  // Normalize vectors
  //////////////////////
  view = bbm::normalize(view);
    
  ////////////////
  // Print Info
  ////////////////
  std::cout << "Plotting '" << plot << "' to file '" << filename << "' with parameters: " << bsdf.toString() << " from " << view << " at: " << width << " x " << height << " resolution";
  if(maskZero && plot == "sample") std::cout << " => Masking samples with zero PDF";
  std::cout << "." << std::endl;
  
  /////////
  // Plot
  /////////
  bitmap<Spectrum> result;
  if(plot == "eval") result = evaluate(bsdf, width, height, view, samples);
  if(plot == "pdf") result = pdf(bsdf, width, height, view, samples);
  if(plot == "sample") result = sample(bsdf, width, height, view, samples, maskZero);
  
  ////////////////
  // Save result
  ////////////////
  std::transform(std::begin(result), std::end(result), std::begin(result), [&scale](auto val) { return val*scale; });
  io::exportPFM(filename, result);
}
