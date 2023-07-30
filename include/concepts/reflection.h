#ifndef _BBM_REFLECTION_CONCEPT_H_
#define _BBM_REFLECTION_CONCEPT_H_

/************************************************************************/
/*! \file refection.h

  \brief attribute and parent class reflection contract.  To do: extend to
  include function and constructor reflection.

*************************************************************************/

namespace bbm {
  namespace concepts {
    namespace reflection {
      
      /******************************************************************/
      /*! \brief attribute reflection allows to enumerate and direct access to
        the attributes of a class. A class supports reflection if the
        following typedef and two methods are implemented:

        + typename attribute_tuple_t                              : a named<tuple> of the types of the attributes (for a non-const struct)
        + named<std::tuple<...>, ...> attribute_tuple(void)       : returns a named<tuple> of references to the attributes
        + named<std::tuple<...>, ...> attribute_tuple(void) const : returns a named<tuple> of const references to the attributes
      *******************************************************************/
      template<typename T>
        concept attributes = requires(T t)
      {
        typename std::decay_t<T>::attribute_tuple_t;
        { t.attribute_tuple() };
        { std::as_const(t).attribute_tuple() };
      };
  

      /******************************************************************/
      /*! \brief basetypes reflection allows to enumerate all base classes.
          The class has the following typename:
        
        + typename reflection_base_t
      *******************************************************************/
      template<typename T>
        concept basetypes = requires
      {
        typename std::decay_t<T>::reflection_base_t;
      };

      /******************************************************************/
      /*! \brief concept to check if a class supports reflections
       ******************************************************************/
      template<typename T>
        concept supported = basetypes<T> || attributes<T>;
      
    } // end reflection namespace
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_REFLECTION_CONCEPT_H_ */
