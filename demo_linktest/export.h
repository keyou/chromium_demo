#ifndef DEMO_LINKTEST_EXPORT_H_
#define DEMO_LINKTEST_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(DEMO_IMPLEMENTATION)
#define DEMO_EXPORT __declspec(dllexport)
#else
#define DEMO_EXPORT __declspec(dllimport)
#endif  // defined(DEMO_IMPLEMENTATION)

#else  // defined(WIN32)
#define DEMO_EXPORT __attribute__((visibility("default")))
#endif

#else  // defined(COMPONENT_BUILD)
#define DEMO_EXPORT
#endif

DEMO_EXPORT void aaa();
DEMO_EXPORT void bbb();

#endif  // DEMO_LINKTEST_EXPORT_H_