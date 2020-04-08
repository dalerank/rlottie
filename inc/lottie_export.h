#ifndef _RLOTTIE_EXPORT_H_
#define _RLOTTIE_EXPORT_H_

#ifdef LOTTIE_STATIC
    #define LOT_EXPORT
#else
    #if defined _WIN32 || defined __CYGWIN__
      #ifdef LOT_BUILD
        #define LOT_EXPORT __declspec(dllexport)
      #else
        #define LOT_EXPORT __declspec(dllimport)
      #endif
    #else
      #ifdef LOT_BUILD
          #define LOT_EXPORT __attribute__ ((visibility ("default")))
      #else
          #define LOT_EXPORT
      #endif
    #endif
#endif

#endif //_RLOTTIE_EXPORT_H_