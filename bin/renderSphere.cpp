#include <iostream>

#include "bbm.h"
#include "bbm/bsdf_import.h"
#include "core/bitmap.h"
#include "core/shading_frame.h"
#include "util/option.h"
#include "io/pfm.h"

using namespace bbm;
BBM_IMPORT_CONFIG( floatRGB );


/***********************************************************************/
/*! \brief geneate a normal map of a sphere
 ***********************************************************************/
bitmap<Vec3d> generateSphereNormalMap(size_t width, size_t height)
{
  bitmap<Vec3d> map(width, height);
  Value radius = std::min(width, height) / 2.0f;
  Vec3d center(width / 2, height /2, 0);
  
  for(size_t y=0; y < height; y++)
    for(size_t x=0; x < width; x++)
    {
      auto& n = map(x,y) = (center - Vec3d(x, y, 0)) / radius;
      Value len = bbm::squared_norm(n);
      if(bbm::any(len < 1.0f)) vec::z(n) = bbm::sqrt(1.0f - len);
      else map(x,y) = 0;
    }
  
  return map;
}


/***********************************************************************/
/*! \brief render the BSDF given a light direction and a normal map
 ***********************************************************************/
bitmap<Spectrum> render(const bsdf_ptr<Config>& bsdf, const Vec3d& lightdir, const Vec3d& viewdir, const bitmap<Vec3d>& normalMap)
{
  bitmap<Spectrum> result(normalMap.width(), normalMap.height(), Spectrum(0));
  
  for(unsigned int y=0; y < result.height(); y++)
    for(unsigned int x=0; x < result.width(); x++)
    {
      const Vec3d& n = normalMap(x,y);
      if(bbm::any(bbm::squared_norm(n) > Constants::Epsilon()))
      {
        auto sf = toLocalShadingFrame(n);
        Vec3d in = sf * lightdir;
        Vec3d out = sf * viewdir;

        result(x,y) = bsdf.eval(in, out)* bbm::abs(vec::z(in));
      }
    }
  
  return result;
}


/***********************************************************************/
/*! \brief main function

  Usage: renderSphere <output.pfm> "python bsdf declarion" [light vec3d] [width=512] [height=512]

  Note: if BBM_PYTHON is not set, the BSDF will be replaced by BBM_BSDF
  instead of being read from python.
  
************************************************************************/
int main(int argc, char** argv)
{
  /////////////////
  // parse input
  /////////////////
  int pos=1;
  if(argc < pos+1)
  {
    std::cout << "Usage: " << argv[0] << " [bsdfmodel=<bsdf string>] [filename=<name>] [light=[0,0,1]] [width=512] [height=512]" << std::endl;
    return -1;
  }

  option_parser opt(argc, argv);

  auto invalid = opt.validate({"bsdfmodel", "filename", "width", "height", "light"});
  if(!invalid.empty())
  {
    std::cout << "ERROR: invalid keywords: " << invalid << "." << std::endl;
    return -1;
  }
  
  auto bsdfmodel = opt.get<std::string>("bsdfmodel");
  auto filename = opt.get<std::string>("filename");
  auto width = opt.get<size_t>("width", 512);
  auto height = opt.get<size_t>("height", 512);
  auto light = opt.get<Vec3d>("light", Vec3d(0,0,-1));

  /////////////////////
  // Validate options
  /////////////////////
  if(filename == "")
  {
    std::cout << "ERROR: expected an output filename." << std::endl;
    return -1;
  }
  
  /////////////////
  // create bsdf
  /////////////////
  auto bsdf = bsdf_import<Config>(bsdfmodel);

  //////////////////////
  // Normalize vectors
  /////////////////////
  light = bbm::normalize(light);
    
  //////////////////////////////////////////////////////
  // print bsdf (directional light and camera direction point towards surface)
  //////////////////////////////////////////////////////
  std::cout << "Rendering with parameters: " << bsdf.toString() << " to file '" << filename << "' with camera direction [0,0,-1] lit with a directional light with direction: " << light << " at: " << width << " x " << height << std::endl;

  /////////////////////////
  // generate normal map
  /////////////////////////
  auto normals = generateSphereNormalMap(width, height);

  //////////////////////////////////////////////////////
  // Render (light and view point _away_ from surface)
  //////////////////////////////////////////////////////
  auto result = render(bsdf, -bbm::normalize(light), Vec3d(0,0,1), normals);
  
  /////////
  // Save
  /////////
  io::exportPFM(filename, result);
}


