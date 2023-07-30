#include <iostream>

#include "bbm.h"
#include "bbm/bsdf_ptr.h"
#include "core/bitmap.h"
#include "core/shading_frame.h"
#include "util/option.h"
#include "io/pfm.h"

#ifdef BBM_PYTHON
  #include "python/bbm_python_interpreter.h"
#elif !defined(BBM_BSDF)
  // Change BBM_BSDF if a different BSDF is needed.
  #define BBM_BSDF lambertian<floatRGB>()
#endif /* BBM_PYTHON */


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
      if(len < 1.0f) vec::z(n) = std::sqrt(1.0f - len);
      else map(x,y) = 0;
    }
  
  return map;
}


/***********************************************************************/
/*! \brief render the BSDF given a light direction and a normal map
 ***********************************************************************/
bitmap<Spectrum> render(const BsdfPtr& bsdf, const Vec3d& lightdir, const Vec3d& viewdir, const bitmap<Vec3d>& normalMap)
{
  bitmap<Spectrum> result(normalMap.width(), normalMap.height(), Spectrum(0));
  
  for(unsigned int y=0; y < result.height(); y++)
    for(unsigned int x=0; x < result.width(); x++)
    {
      const Vec3d& n = normalMap(x,y);
      if(bbm::squared_norm(n) > Constants::Epsilon())
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
    std::cout << "Usage: " << argv[0] << " [bsdfmodel=<python string>] [filename=<name>] [light=[0,0,1]] [width=512] [height=512]" << std::endl;
    #ifndef BBM_PYTHON
    std::cout << "BBM compiled without Python support. <bsdfmodel> is ignored and " << BBM_STRINGIFY(BBM_BSDF) << " is used instead." << std::endl;
    #endif
    return -1;
  }

  auto opt = parse_options(argc, argv);
  
  auto bsdfmodel = get<string_option>(opt, "bsdfmodel");
  auto filename = get<string_option>(opt, "filename");
  auto width = get<int_option>(opt, "width", 512);
  auto height = get<int_option>(opt, "height", 512);
  auto light = get<vec3d_option<float_option>>(opt, "light", Vec3d(0,0,-1));

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
  #ifdef BBM_PYTHON
    if(bsdfmodel == "")
    {
      std::cout << "ERROR: bsdfmodel not speficied." << std::endl;
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


