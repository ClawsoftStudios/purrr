//
// Created by kopec on 6/18/2025.
//

#ifndef EXPORT_H
#define EXPORT_H

#ifdef _WIN32
  #ifdef PURRR_BUILD_DLL
    #define EXPORT __declspec(dllexport)
  #else
    #define EXPORT __declspec(dllimport)
  #endif
#else
  #define EXPORT __attribute__((visibility("default")))
#endif

#endif //EXPORT_H
