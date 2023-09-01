// Mitsuba Includes
#include <mitsuba/render/bsdf.h>
#include <mitsuba/hw/basicshader.h>
#include <mitsuba/core/warp.h>

// check if BBM is properly setup
#ifndef BBM_CONFIG
  #define BBM_CONFIG floatRGB
#endif

// Undef the mitsuba Epsilon macro; clashes with bbm::constant<T>::Epsilon()
#undef Epsilon  

// BBM Includes
#include "bbm.h"
#include "bbm/bsdf_import.h"

MTS_NAMESPACE_BEGIN

/***********************************************************************/
/*! \file mitsuba_bbm_plugin.cpp
    \brief Allow to use BBM bsdfs in Mitsuba 0.6.0

    This file must be compiled as a Mitsuba plugin.  This plugin uses the bbm
    bsdf string format to specify a BSDF.  While this causes some overhead,
    mainly an extra 'vritual' function call when evaluating the BSDF, it
    alleviates the plugin from managing the different BSDF
    model. Alternatively, one could create a plugin per BBM bsdf model, at the
    cost of creating many plugins.

    Assumes BBM_CONFIG is set.
    
    Note: this plug-in will use embedded python when the bsdf_import method
    uses python as the interpreter to avoid locating and reading in the python
    lib.

    Usage:

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.xml}
    <bsdf type="mitsuba_bbm">
      <string name="bsdf" value="<bsdf string>"/>
    </bsdf>
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    where '<pythin definiton>' can be any python code that returns a BsdfPtr.
    The relevant bbm python module is already imported.  For example:

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.xml}
    <bsdf type="mitsuba_bbm">
      <string name="bsdf" value="Lambertian([0.3, 0.6, 0.9])"/>
    </bsdf>
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    creates a 'Lambertian' BSDF with 'diffuseAlbedo = [0.3, 0.6, 0.9]'.

    **Key differences between Mitsuba and BBM**:

    - Mitsuba uses the convention that 'wo' is sampled vs. BBM uses 'in' is
      sampled.

    - Mitsuba uses the convention that 'wo' points towards the light source
      vs. in BBM 'in' points towards the light source.

    - Mitsuba includes foreshortning in eval vs. BBM does not.
*************************************************************************/
class BBMbsdf : public BSDF
{
  
public:
  BBMbsdf(const Properties &props) : BSDF(props)
  { 
    bbm_str = props.getString("bsdf", "Lambertian()");

    // Notify which BSDF is created
    SLog(EInfo, "Using BBM bsdf: \"%s\"", bbm_str.c_str());
  }

  BBMbsdf(Stream *stream, InstanceManager *manager) : BSDF(stream, manager)
  {
    bbm_str = stream->readString();
    
    // configure
    configure();
  }

  void configure()
  {
    // Mitsuba setup 
    m_components.clear();
    m_components.push_back(EDiffuseReflection | EFrontSide );
    m_components.push_back(EGlossyReflection | EFrontSide );
    m_usesRayDifferentials = false;

    // create BSDF
    bbm_bsdf_ptr = bbm::bsdf_import<bbm::BBM_CONFIG>(bbm_str);

    // Configure
    BSDF::configure();
  }

  
  Spectrum eval(const BSDFSamplingRecord &bRec, EMeasure measure) const
  {
    if(measure != ESolidAngle || 
       Frame::cosTheta(bRec.wi) <= 0 ||
       Frame::cosTheta(bRec.wo) <= 0)
      return Spectrum(0.0f);

    /* set flags */
    auto flags = bbm::bsdf_flag::None;
    if((bRec.typeMask & EDiffuseReflection) && (bRec.component == -1 || bRec.component == 0)) flags |= bbm::bsdf_flag::Diffuse;
    if((bRec.typeMask & EGlossyReflection) && (bRec.component == -1 || bRec.component == 1)) flags |= bbm::bsdf_flag::Specular;
    
    /* set unit */
    auto unit = bbm::unit_t::Radiance;
    if(bRec.mode & EImportance) unit = bbm::unit_t::Importance;

    /* eval BSDF */
    Spectrum result(0.0f);
    bbm::Vec3d_t<bbm::BBM_CONFIG> in(bRec.wo.x, bRec.wo.y, bRec.wo.z);
    bbm::Vec3d_t<bbm::BBM_CONFIG> out(bRec.wi.x, bRec.wi.y, bRec.wi.z);
    auto bbm_eval = bbm_bsdf_ptr->eval(in, out, flags, unit);
    
    /* convert solution to Mitsuba Spectrum */
    result.fromLinearRGB(bbm::cast<Float>(bbm_eval[0]), bbm::cast<Float>(bbm_eval[1]), bbm::cast<Float>(bbm_eval[2]));

    /* TODO: single channel or multi-spectrum not implemented yet */
	
    // Done.
    return result * Frame::cosTheta(bRec.wo);
  }

  Float pdf(const BSDFSamplingRecord &bRec, EMeasure measure) const
  {
    if (measure != ESolidAngle ||
	Frame::cosTheta(bRec.wi) <= 0 ||
	Frame::cosTheta(bRec.wo) <= 0)
      return 0.0f;

    /* set flags */
    auto flags = bbm::bsdf_flag::None;
    if((bRec.typeMask & EDiffuseReflection) && (bRec.component == -1 || bRec.component == 0)) flags |= bbm::bsdf_flag::Diffuse;
    if((bRec.typeMask & EGlossyReflection) && (bRec.component == -1 || bRec.component == 1)) flags |= bbm::bsdf_flag::Specular;

    /* set unit */
    auto unit = bbm::unit_t::Radiance;
    if(bRec.mode & EImportance) unit = bbm::unit_t::Importance;

    /* eval PDF */
    bbm::Vec3d_t<bbm::BBM_CONFIG> in(bRec.wo.x, bRec.wo.y, bRec.wo.z);
    bbm::Vec3d_t<bbm::BBM_CONFIG> out(bRec.wi.x, bRec.wi.y, bRec.wi.z);    
    auto bbm_pdf = bbm::cast<Float>(bbm_bsdf_ptr->pdf(in, out, flags, unit));

    // Done.
    return bbm_pdf;
  }

  Spectrum sample(BSDFSamplingRecord &bRec, Float &pdf, const Point2 &sample) const
  {
    /* set flags */
    auto flags = bbm::bsdf_flag::None;
    if((bRec.typeMask & EDiffuseReflection) && (bRec.component == -1 || bRec.component == 0)) flags |= bbm::bsdf_flag::Diffuse;
    if((bRec.typeMask & EGlossyReflection) && (bRec.component == -1 || bRec.component == 1)) flags |= bbm::bsdf_flag::Specular;

    /* set unit */
    auto unit = bbm::unit_t::Radiance;
    if(bRec.mode & EImportance) unit = bbm::unit_t::Importance;
    
    /* sample */
    bbm::Vec3d_t<bbm::BBM_CONFIG> out(bRec.wi.x, bRec.wi.y, bRec.wi.z);
    bbm::Vec2d_t<bbm::BBM_CONFIG> xi(sample.x, sample.y);
    auto bbm_sample = bbm_bsdf_ptr->sample(out, xi, flags, unit);

    /* Fill in sample record */
    pdf = bbm::cast<Float>(bbm_sample.pdf);
    bRec.wo = Vector(bbm::cast<Float>(bbm_sample.direction[0]), bbm::cast<Float>(bbm_sample.direction[1]), bbm::cast<Float>(bbm_sample.direction[2]));
    bRec.sampledComponent = bbm::select(bbm::is_set(bbm_sample.flag, bbm::bsdf_flag::Diffuse), 0, 1);
    bRec.sampledType = bbm::select(bbm::is_set(bbm_sample.flag, bbm::bsdf_flag::Diffuse), EDiffuseReflection, EGlossyReflection);
       
    /* eval spectrum */
    if(pdf == 0 || Frame::cosTheta(bRec.wo) <= 0)
      return Spectrum(0.0f);
    else
      return eval(bRec, ESolidAngle) / pdf;
  }

  Spectrum sample(BSDFSamplingRecord &bRec, const Point2 &sample) const
  {
    Float pdf;
    return BBMbsdf::sample(bRec, pdf, sample);
  }

  void serialize(Stream *stream, InstanceManager *manager) const
  {
    BSDF::serialize(stream, manager);
    stream->writeString(bbm_str);
  }

  Float getRoughness(const Intersection& /*its*/, int /*component*/) const
  {
    // TODO
    return 0.0f;
  }

  std::string toString() const
  {
    std::ostringstream oss;
    oss << "BBMbsdf[" << " bsdf = \"" << bbm_str << "\"]";
    return oss.str();
  }

  Shader *createShader(Renderer *renderer) const;

  MTS_DECLARE_CLASS()
private:
  // attribtues
  std::string bbm_str;
  bbm::bsdf_ptr<bbm::BBM_CONFIG> bbm_bsdf_ptr;
};

// ================ Hardware shader implementation ================

/* TODO: shader-- render as a 'black box' */
class BBMbsdfShader : public Shader {
public:
	BBMbsdfShader(Renderer *renderer) :
		Shader(renderer, EBSDFShader) {
		m_flags = ETransparent;
	}

	void generateCode(std::ostringstream &oss,
			const std::string &evalName,
                        const std::vector<std::string>& /*depNames*/) const {
		oss << "vec3 " << evalName << "(vec2 uv, vec3 wi, vec3 wo) {" << endl
			<< "    return vec3(0.0);" << endl
			<< "}" << endl;
		oss << "vec3 " << evalName << "_diffuse(vec2 uv, vec3 wi, vec3 wo) {" << endl
			<< "    return vec3(0.0);" << endl
			<< "}" << endl;
	}
	MTS_DECLARE_CLASS()
};

Shader *BBMbsdf::createShader(Renderer *renderer) const {
	return new BBMbsdfShader(renderer);
}

MTS_IMPLEMENT_CLASS(BBMbsdfShader, false, Shader)
MTS_IMPLEMENT_CLASS_S(BBMbsdf, false, BSDF)
MTS_EXPORT_PLUGIN(BBMbsdf, "BBM BSDF");
MTS_NAMESPACE_END
