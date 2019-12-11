/**
 * \file platform_util.h
 *
 * \brief Common and shared functions used by multiple modules in the Mbed TLS
 *        library.
 */
/*
 *  Copyright (C) 2018, Arm Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of Mbed TLS (https://tls.mbed.org)
 */
#ifndef MBEDTLS_PLATFORM_UTIL_H
#define MBEDTLS_PLATFORM_UTIL_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
#include <stdint.h>
#include <stddef.h>
#if defined(MBEDTLS_HAVE_TIME_DATE)
#include "platform_time.h"
#include <time.h>
#endif /* MBEDTLS_HAVE_TIME_DATE */

#include "hmac_drbg.h"
#include "ctr_drbg.h"
#include "entropy.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MBEDTLS_CHECK_PARAMS)

#if defined(MBEDTLS_CHECK_PARAMS_ASSERT)
/* Allow the user to define MBEDTLS_PARAM_FAILED to something like assert
 * (which is what our config.h suggests). */
#include <assert.h>
#endif /* MBEDTLS_CHECK_PARAMS_ASSERT */

#if defined(MBEDTLS_PARAM_FAILED)
/** An alternative definition of MBEDTLS_PARAM_FAILED has been set in config.h.
 *
 * This flag can be used to check whether it is safe to assume that
 * MBEDTLS_PARAM_FAILED() will expand to a call to mbedtls_param_failed().
 */
#define MBEDTLS_PARAM_FAILED_ALT

#elif defined(MBEDTLS_CHECK_PARAMS_ASSERT)
#define MBEDTLS_PARAM_FAILED( cond ) assert( cond )
#define MBEDTLS_PARAM_FAILED_ALT

#else /* MBEDTLS_PARAM_FAILED */
#define MBEDTLS_PARAM_FAILED( cond ) \
    mbedtls_param_failed( #cond, __FILE__, __LINE__ )

/**
 * \brief       User supplied callback function for parameter validation failure.
 *              See #MBEDTLS_CHECK_PARAMS for context.
 *
 *              This function will be called unless an alternative treatement
 *              is defined through the #MBEDTLS_PARAM_FAILED macro.
 *
 *              This function can return, and the operation will be aborted, or
 *              alternatively, through use of setjmp()/longjmp() can resume
 *              execution in the application code.
 *
 * \param failure_condition The assertion that didn't hold.
 * \param file  The file where the assertion failed.
 * \param line  The line in the file where the assertion failed.
 */
void mbedtls_param_failed( const char *failure_condition,
                           const char *file,
                           int line );
#endif /* MBEDTLS_PARAM_FAILED */

/* Internal macro meant to be called only from within the library. */
#define MBEDTLS_INTERNAL_VALIDATE_RET( cond, ret )  \
    do {                                            \
        if( !(cond) )                               \
        {                                           \
            MBEDTLS_PARAM_FAILED( cond );           \
            return( ret );                          \
        }                                           \
    } while( 0 )

/* Internal macro meant to be called only from within the library. */
#define MBEDTLS_INTERNAL_VALIDATE( cond )           \
    do {                                            \
        if( !(cond) )                               \
        {                                           \
            MBEDTLS_PARAM_FAILED( cond );           \
            return;                                 \
        }                                           \
    } while( 0 )

#else /* MBEDTLS_CHECK_PARAMS */

/* Internal macros meant to be called only from within the library. */
#define MBEDTLS_INTERNAL_VALIDATE_RET( cond, ret )  do { } while( 0 )
#define MBEDTLS_INTERNAL_VALIDATE( cond )           do { } while( 0 )

#endif /* MBEDTLS_CHECK_PARAMS */

#if defined(__GNUC__) || defined(__arm__)
#define MBEDTLS_ALWAYS_INLINE __attribute__((always_inline))
#else
#define MBEDTLS_ALWAYS_INLINE
#endif

/* Internal helper macros for deprecating API constants. */
#if !defined(MBEDTLS_DEPRECATED_REMOVED)
#if defined(MBEDTLS_DEPRECATED_WARNING)
/* Deliberately don't (yet) export MBEDTLS_DEPRECATED here
 * to avoid conflict with other headers which define and use
 * it, too. We might want to move all these definitions here at
 * some point for uniformity. */
#define MBEDTLS_DEPRECATED __attribute__((deprecated))
MBEDTLS_DEPRECATED typedef char const * mbedtls_deprecated_string_constant_t;
#define MBEDTLS_DEPRECATED_STRING_CONSTANT( VAL )       \
    ( (mbedtls_deprecated_string_constant_t) ( VAL ) )
MBEDTLS_DEPRECATED typedef int mbedtls_deprecated_numeric_constant_t;
#define MBEDTLS_DEPRECATED_NUMERIC_CONSTANT( VAL )       \
    ( (mbedtls_deprecated_numeric_constant_t) ( VAL ) )
#undef MBEDTLS_DEPRECATED
#else /* MBEDTLS_DEPRECATED_WARNING */
#define MBEDTLS_DEPRECATED_STRING_CONSTANT( VAL ) VAL
#define MBEDTLS_DEPRECATED_NUMERIC_CONSTANT( VAL ) VAL
#endif /* MBEDTLS_DEPRECATED_WARNING */
#endif /* MBEDTLS_DEPRECATED_REMOVED */

/**
 * \brief       Securely zeroize a buffer
 *
 *              The function is meant to wipe the data contained in a buffer so
 *              that it can no longer be recovered even if the program memory
 *              is later compromised. Call this function on sensitive data
 *              stored on the stack before returning from a function, and on
 *              sensitive data stored on the heap before freeing the heap
 *              object.
 *
 *              It is extremely difficult to guarantee that calls to
 *              mbedtls_platform_zeroize() are not removed by aggressive
 *              compiler optimizations in a portable way. For this reason, Mbed
 *              TLS provides the configuration option
 *              MBEDTLS_PLATFORM_ZEROIZE_ALT, which allows users to configure
 *              mbedtls_platform_zeroize() to use a suitable implementation for
 *              their platform and needs
 *
 * \param buf   Buffer to be zeroized
 * \param len   Length of the buffer in bytes
 *
 */
void mbedtls_platform_zeroize( void *buf, size_t len );

/**
 * \brief       Secure memset
 *
 *              This is a constant-time version of memset(). The buffer is
 *              initialised with random data and the order is also
 *              randomised in order to further harden against side-channel attacks.
 *
 * \param ptr   Buffer to be set.
 * \param value Value to be used when setting the buffer.
 * \param num   The length of the buffer in bytes.
 *
 * \return      The value of \p ptr.
 */
void *mbedtls_platform_memset( void *ptr, int value, size_t num );

/**
 * \brief       Secure memcpy
 *
 *              This is a constant-time version of memcpy(). The buffer is
 *              initialised with random data and the order is also
 *              randomised in order to further harden against side-channel attacks.
 *
 * \param dst   Destination buffer where the data is being copied to.
 * \param src   Source buffer where the data is being copied from.
 * \param num   The length of the buffers in bytes.
 *
 * \return      The value of \p dst.
 */
void *mbedtls_platform_memcpy( void *dst, const void *src, size_t num );

/**
 * \brief       Secure memcmp
 *
 *              This is a constant-time version of memcmp(). Order is
 *              randomised  in order to further harden
 *              against side-channel attacks.
 *
 * \param buf1  First buffer to compare.
 * \param buf2  Second buffer to compare against.
 * \param num   The length of the buffers in bytes.
 *
 * \return      0 if the buffers were equal or an unspecified non-zero value
 *              otherwise.
 */
int mbedtls_platform_memcmp( const void *buf1, const void *buf2, size_t num );

/**
 * \brief       RNG-function for getting a random in given range.
 *
 *              This function is meant to provide a global RNG to be used
 *              throughout Mbed TLS for hardening the library. It is used
 *              for generating a random delay, random data or random offset
 *              for utility functions. It is not meant to be a
 *              cryptographically secure RNG, but provide an RNG for utility
 *              functions.
 *
 * \note        If the given range is [0, 0), 0 is returned.
 *
 * \param num   Max-value for the generated random number, exclusive.
 *              The generated number will be on range [0, num).
 *
 * \return      The generated random number.
 */
uint32_t mbedtls_platform_random_in_range( size_t num );

/**
 * \brief       This function does nothing, but can be inserted between
 *              successive reads to a volatile local variable to prevent
 *              compilers from optimizing them away.
 */
void mbedtls_platform_enforce_volatile_reads( void );

#if defined(MBEDTLS_HAVE_TIME_DATE)
/**
 * \brief      Platform-specific implementation of gmtime_r()
 *
 *             The function is a thread-safe abstraction that behaves
 *             similarly to the gmtime_r() function from Unix/POSIX.
 *
 *             Mbed TLS will try to identify the underlying platform and
 *             make use of an appropriate underlying implementation (e.g.
 *             gmtime_r() for POSIX and gmtime_s() for Windows). If this is
 *             not possible, then gmtime() will be used. In this case, calls
 *             from the library to gmtime() will be guarded by the mutex
 *             mbedtls_threading_gmtime_mutex if MBEDTLS_THREADING_C is
 *             enabled. It is recommended that calls from outside the library
 *             are also guarded by this mutex.
 *
 *             If MBEDTLS_PLATFORM_GMTIME_R_ALT is defined, then Mbed TLS will
 *             unconditionally use the alternative implementation for
 *             mbedtls_platform_gmtime_r() supplied by the user at compile time.
 *
 * \param tt     Pointer to an object containing time (in seconds) since the
 *               epoch to be converted
 * \param tm_buf Pointer to an object where the results will be stored
 *
 * \return      Pointer to an object of type struct tm on success, otherwise
 *              NULL
 */
struct tm *mbedtls_platform_gmtime_r( const mbedtls_time_t *tt,
                                      struct tm *tm_buf );
#endif /* MBEDTLS_HAVE_TIME_DATE */

/**
 * \brief      Convert a 32-bit number to the big endian format and write it to
 *             the given buffer.
 *
 * \param buf  Address where the converted number is written.
 * \param num  A number that needs to be converted to the big endian format.
 *
 * \return     Address to the end of buffer where the converted number is
 *             written.
  */
unsigned char* mbedtls_platform_put_uint32_be( unsigned char *buf,
                                               size_t num );

/**
 * \brief      Convert a 24-bit number to the big endian format and write it to
 *             the given buffer.
 *
 * \param buf  Address where the converted number is written.
 * \param num  A number that needs to be converted to the big endian format.
 *
 * \return     Address to the end of buffer where the converted number is
 *             written.
  */
unsigned char* mbedtls_platform_put_uint24_be( unsigned char *buf,
                                               size_t num );

/**
 * \brief      Convert a 16-bit number to the big endian format and write it to
 *             the given buffer.
 *
 *
 * \param buf  Address where the converted number is written.
 * \param num  A number that needs to be converted to the big endian format.
 *
 * \return     Address to the end of buffer where the converted number is
 *             written.
  */
unsigned char* mbedtls_platform_put_uint16_be( unsigned char *buf,
                                               size_t num );

/**
 * \brief      Convert a 32-bit number from the big endian format.
 *
 *             The function reads a 32-bit number from the given buffer in the
 *             big endian format and returns it to the caller.
 *
 * \param buf  Buffer where the 32-bit number locates.
 *
 * \return     Converted number.
 */
size_t mbedtls_platform_get_uint32_be( const unsigned char *buf );

/**
 * \brief      Convert a 24-bit number from the big endian format.
 *
 *             The function reads a 14-bit number from the given buffer in the
 *             big endian format and returns it to the caller.
 *
 * \param buf  Buffer where the 24-bit number locates.
 *
 * \return     Converted number.
 */
size_t mbedtls_platform_get_uint24_be( const unsigned char *buf );

/**
 * \brief      Convert a 16-bit number from the big endian format.
 *
 *             The function reads a 16-bit number from the given buffer in the
 *             big endian format and returns it to the caller.
 *
 * \param buf  Buffer where the 16-bit number locates.
 *
 * \return     Converted number.
 */
size_t mbedtls_platform_get_uint16_be( const unsigned char *buf );



#if !defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

#if defined(MBEDTLS_HMAC_DRBG_C)
    mbedtls_entropy_context _entropy;
    mbedtls_hmac_drbg_context _drbg;
#elif defined(MBEDTLS_CTR_DRBG_C)
    mbedtls_entropy_context _entropy;
    mbedtls_ctr_drbg_context _drbg;
#endif /* MBEDTLS_HMAC_DRBG_C */

#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_PLATFORM_UTIL_H */
