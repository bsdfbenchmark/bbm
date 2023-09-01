#include <iostream>
#include <random>

#include "bbm.h"
#include "bbm/bsdf_import.h"
#include "core/bitmap.h"
#include "util/option.h"
#include "io/pfm.h"


using namespace bbm;
BBM_IMPORT_CONFIG( floatRGB );


bitmap<Spectrum> evaluate(const bsdf_ptr<Config>& bsdf, size_t width, size_t height, const Vec3d& view, size_t samples)
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


bitmap<Spectrum> pdf(const bsdf_ptr<Config>& bsdf, size_t width, size_t height, const Vec3d& view, size_t samples)
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


bitmap<Spectrum> sample(const bsdf_ptr<Config>& bsdf, size_t width, size_t height, const Vec3d& view, size_t samples, bool maskZero)
{
  std::mt19937 rnd;
  std::uniform_real_distribution<float> U(0,1);
  bitmap<Spectrum> result(width, height);
  
  for(size_t s=0; s < samples*width*height; ++s)
  {
    auto sample = bsdf->sample(view, Vec2d(U(rnd), U(rnd)));
    Vec2d sph_coord = spherical::convert(sample.direction);

    size_t x = bbm::cast<size_t>(bbm::min( spherical::phi(sph_coord) / Constants::Pi(2) * width, width-1));
    size_t y = bbm::cast<size_t>(bbm::min( spherical::theta(sph_coord) / Constants::Pi() * height, height-1));

    if(!maskZero || bbm::any(sample.pdf > Constants::Epsilon()))
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
    std::cout << "Usage: " << argv[0] << " [bsdfmodel=<bsdf string>] [filename=<name>] [width=512] [height=256] [view=[0,0,1]] [samples=1] [scale=1] [maskZero] [plot=<eval|pdf|sample>]" << std::endl;
    return -1;
  }

  option_parser opt(argc, argv);

  auto invalid = opt.validate({"bsdfmodel", "filename", "width", "height", "view", "samples", "scale", "maskZero", "plot"});
  if(!invalid.empty())
  {
    std::cout << "ERROR: invalid keywords: " << invalid << "." << std::endl;
    return -1;
  }
  
  std::string bsdfmodel = opt.get<std::string>("bsdfmodel");
  std::string filename = opt.get<std::string>("filename");
  size_t width = opt.get<size_t>("width", 512);
  size_t height = opt.get<size_t>("height", 256);
  size_t samples = opt.get<size_t>("samples", 1);
  Vec3d view = opt.get<Vec3d>("view", Vec3d(0,0,1));
  std::string plot = opt.get<std::string>("plot");
  float scale = opt.get<float>("scale", 1.0);
  bool maskZero = opt.get<bool>("maskZero", false);
  
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
  auto bsdf = bsdf_import<Config>(bsdfmodel);

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
