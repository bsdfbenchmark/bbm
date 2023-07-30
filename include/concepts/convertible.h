#ifndef _BBM_CONVERTIBLE_CONCEPT_H_
#define _BBM_CONVERTIBLE_CONCEPT_H_

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief convertible concept

      Check if convert(DST& dst, const SRC& src) exists.  The goal of this
      method is to copy and convert the data from 'src' to 'dst'.  Essentially
      an extra-class cast method.
    *********************************************************************/
    template<typename DST, typename SRC>
      concept convertible = requires(DST& dst, const SRC& src)
    {
      { convert(dst, src) };
    };

  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_CONVERTIBLE_CONCEPT_H_ */
