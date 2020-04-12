#ifndef INCLUDED_ZOE_DEFS_H
#define INCLUDED_ZOE_DEFS_H

/*---------------------------------------------------------------------------*
 * Common Definitions
 *---------------------------------------------------------------------------*/

#if defined(__GNUC__)
#  define ALIGN4    __attribute__((aligned(4)))
#  define ALIGN8    __attribute__((aligned(8)))
#  define ALIGN16   __attribute__((aligned(16)))
#  define ALIGN32   __attribute__((aligned(32)))
#  define ALIGN64   __attribute__((aligned(64)))
#  define ALIGN128  __attribute__((aligned(128)))
#  define ALIGN(x)  __attribute__((aligned(x)))
#else
#  define ALIGN4
#  define ALIGN8
#  define ALIGN16
#  define ALIGN32
#  define ALIGN64
#  define ALIGN128
#  define ALIGN(x)
#endif

// Hack for unknown const values.
// TODO: Replace anything initialized to TEMP_ZERO with the real values.
//       Delete this definition once all instances have been replaced.
#define TEMP_ZERO (0)

/*---------------------------------------------------------------------------*
 * Configuration Definitions
 *---------------------------------------------------------------------------*/

#if (ZOE_JPN)
// dummy_printf() is defined in include/orgtype.h according to .mdebug
void dummy_printf( const char *fmt, ... );

// dummy printf for non-debug builds
//#define printf(fmt, ...)  dummy_printf(fmt, ##__VA_ARGS__)
#define printf  dummy_printf

#endif // ZOE_JPN

#endif /* END OF FILE */
