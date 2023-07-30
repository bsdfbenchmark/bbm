#ifndef _BBM_DYNAMIC_LIBRARY_H_
#define _BBM_DYNAMIC_LIBRARY_H_

#include <string>

/***********************************************************************/
/*! \file dynamic_library.h
    \brief load and unload a dynamic library
************************************************************************/

#ifdef __WINDOWS__
   #include <windows.h>

   namespace bbm {

       using dl_handle_t = HMODULE;
  
       inline dl_handle_t loadDynamicLibrary(const std::string& name)
       {
         return LoadLibraryW(name.c_str());
       }

       inline void closeDynamicLibrary(dl_handle_t handle)
       {
         FreeLibrary(handle);
       }
     
   } // end bbm namespace
     
#else
   #include <dlfcn.h>

   namespace bbm {
     
       using dl_handle_t = void*;
   
       inline dl_handle_t loadDynamicLibrary(const std::string& name)
       {
         return dlopen(name.c_str(), RTLD_LAZY | RTLD_GLOBAL);
       }

       inline void closeDynamicLibrary(dl_handle_t handle)
       {
         dlclose(handle);
       }

   } // end bbm namespace

#endif
   
#endif /* _BBM_DYNAMIC_LIBRARY_H_ */
