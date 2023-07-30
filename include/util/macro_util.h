#ifndef _BBM_MACRO_UTIL_H_
#define _BBM_MACRO_UTIL_H_

/***********************************************************************/
/*! \file macro_util.h
  \brief General macro utilities.
************************************************************************/

#define BBM_PARENS ()

/***********************************************************************/
/*! \brief Just pass the arguments
************************************************************************/
#define BBM_PASS(...) __VA_ARGS__

/***********************************************************************/
/*! \brief Expand the args, then call the macro
************************************************************************/
#define BBM_CALL(macro, ...)  macro (__VA_ARGS__)

/***********************************************************************/
/*! \brief A comma
************************************************************************/
#define BBM_COMMA() ,

/***********************************************************************/
/*! \brief Re-scan __VA_ARGS__ 342 times. Part of BBM_FOREACH.
************************************************************************/
#define   BBM_EXPAND(...) _BBM_EXPAND4(_BBM_EXPAND4(_BBM_EXPAND4(_BBM_EXPAND4(__VA_ARGS__))))
#define _BBM_EXPAND4(...) _BBM_EXPAND3(_BBM_EXPAND3(_BBM_EXPAND3(_BBM_EXPAND3(__VA_ARGS__))))
#define _BBM_EXPAND3(...) _BBM_EXPAND2(_BBM_EXPAND2(_BBM_EXPAND2(_BBM_EXPAND2(__VA_ARGS__))))
#define _BBM_EXPAND2(...) _BBM_EXPAND1(_BBM_EXPAND1(_BBM_EXPAND1(_BBM_EXPAND1(__VA_ARGS__))))
#define _BBM_EXPAND1(...) __VA_ARGS__


/***********************************************************************/
/*! \brief A macro that applies a macro (MACRO) on every argument in
  __VA_ARGS__ followed by an (OPT) macro (without arguments) except following
  the last argument in __VA_ARGS__.

  This code is based on https://www.scs.stanford.edu/~dm/blog/va-opt.html
  
  \param MACRO = macro to apply to each argument in __VA_ARGS__ (takes one argument)
  \param OPT = macro to apply between each argument in __VA_ARGS__ (takes no arguments)
  \param ... = list of __VA_ARGS__
*************************************************************************/
#define BBM_FOREACH_OPT(MACRO, OPT, ...)                                 \
  __VA_OPT__(BBM_EXPAND(_BBM_FOREACH_OPT(MACRO, OPT, __VA_ARGS__)))
#define _BBM_FOREACH_OPT(MACRO, OPT, ARG, ...)                           \
  MACRO(ARG)                                                             \
  __VA_OPT__(OPT() _BBM_CALL_FOREACH_OPT BBM_PARENS (MACRO, OPT, __VA_ARGS__))
#define _BBM_CALL_FOREACH_OPT() _BBM_FOREACH_OPT

/***********************************************************************/
/*! \brief A macro that applies another macro (MACRO) on every argument in
 __VA_ARGS__ (i.e., BBM_FOREACH_OPT with empty OPT-macro)
  
  \param MACRO = macro to apply to each
  \param ... = list of __VA_ARGS__
*************************************************************************/
#define BBM_FOR_EACH(MACRO, ...)  BBM_FOREACH_OPT(MACRO, , __VA_ARGS__)


/***********************************************************************/
/*! \brief __VA_ARGS__ => "__VA_ARGS__"
************************************************************************/
#define BBM_TO_STRING(...)  #__VA_ARGS__

/***********************************************************************/
/*! \brief Expand __VA_ARGS__ before converting to a string of comma separated elements

  BBM_STRINGIFY(a,b,c) => "a, b, c"
************************************************************************/
#define BBM_STRINGIFY(...) BBM_TO_STRING(__VA_ARGS__)

/***********************************************************************/
/*! \brief Expand __VA_ARGS__ before converting to a comma separated series strings of elements

  BBM_STRINGIFY_EACH(a,b,c) => "a", "b", "c"
************************************************************************/
#define BBM_STRINGIFY_EACH(...) BBM_FOREACH_OPT(BBM_TO_STRING, BBM_COMMA, __VA_ARGS__)

#endif /* _BBM_MACRO_UTIL_H_ */
