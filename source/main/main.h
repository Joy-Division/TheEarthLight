#ifndef INCLUDED_MAIN_H
#define INCLUDED_MAIN_H

//
// The following COP0 Cache definitions were removed from eekernel.h in
// version 2.3.0 (Jan.22,2003), however the functions are still present
// within the libkernl.a static library.
//
#if defined(_eekernel_h_)

#ifndef INST_CACHE
#define INST_CACHE	2
#endif
#ifndef DATA_CACHE
#define DATA_CACHE	1
#endif

#ifndef CacheOn
#define CacheOn()	EnableCache(INST_CACHE | DATA_CACHE)
#endif
#ifndef CacheOff
#define CacheOff()	DisableCache(INST_CACHE | DATA_CACHE)
#endif

extern "C" int EnableCache(int);
extern "C" int DisableCache(int);

#endif // _eekernel_h_

#endif /* END OF FILE */
