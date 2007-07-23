#ifndef _MEDIATE_ANALYSIS_WINDOW_H_GUARD
#define _MEDIATE_ANALYSIS_WINDOW_H_GUARD

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// mediate_analysis_window_t
//
// Contains all user-specified information about a single spectral analysis window.
// It allows the GUI to provide information to the engine.

typedef struct mediate_analysis_window
{
  // To Be Defined - Coupled to the control offered by the GUI Analysis Window Tabs and
  // the list of molecules/cross-sections/symbols?

  int todo;

} mediate_analysis_window_t;


#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
