/*
 * tochars.hh
 *
 *  Created on: 13.11.2011
 *      Author: 0xA5EA
 */

#ifndef TOCHARS_HH_
#define TOCHARS_HH_

#include <cstdio>
#include <cstddef> // fuer std::ptrdiff_t

#if (defined(WIN32) && !defined(__MINGW32__)) || (defined (__WIN64__) && !defined(__MINGW32__))
#define snprintf sprintf_s
#endif


namespace kpq3
{
  namespace impl
  {
    #define TOCHARS_TRAIT_SPECIAL_DEF(type, fmt) template<> struct ToCharFmt<type> {  static inline char const* Fmt() throw() { return fmt; } }
    #define TOCHARS_TRAIT_PARTIAL_DEF(type, qual, fmt) template<typename type> struct ToCharFmt<type qual> {  static inline char const* Fmt() throw() { return fmt; } }

    template<typename T>
    struct ToCharFmt;

    TOCHARS_TRAIT_PARTIAL_DEF( T, *,                 "%p" );
    TOCHARS_TRAIT_PARTIAL_DEF( T, *&,                "%p" );
    TOCHARS_TRAIT_SPECIAL_DEF(               char,   "%c" );
    TOCHARS_TRAIT_SPECIAL_DEF( unsigned      char,   "%u" );
    TOCHARS_TRAIT_SPECIAL_DEF(   signed      char,   "%d" );
    TOCHARS_TRAIT_SPECIAL_DEF(              short,   "%d" );
    TOCHARS_TRAIT_SPECIAL_DEF( unsigned     short,   "%u" );
    TOCHARS_TRAIT_SPECIAL_DEF(                int,   "%d" );
    TOCHARS_TRAIT_SPECIAL_DEF( unsigned       int,   "%u" );
    TOCHARS_TRAIT_SPECIAL_DEF( unsigned long long, "%llu" );
    TOCHARS_TRAIT_SPECIAL_DEF(          long long, "%lld" );
    TOCHARS_TRAIT_SPECIAL_DEF(               float,  "%f" );
    TOCHARS_TRAIT_SPECIAL_DEF(              double,  "%f" );
    TOCHARS_TRAIT_SPECIAL_DEF(         long double, "%Lf" );
#ifndef _LP64
    TOCHARS_TRAIT_SPECIAL_DEF(               long,  "%ld" );
    TOCHARS_TRAIT_SPECIAL_DEF( unsigned      long,  "%lu" );
#else
    TOCHARS_TRAIT_SPECIAL_DEF( unsigned      long,  "%zu" );
    TOCHARS_TRAIT_SPECIAL_DEF(               long,  "%zd" );
#endif

    template<typename T>
    struct ToCharsImpl
    {
      template<typename Char, std::size_t N>
      static inline void Doit(Char(&buf)[N], T a) throw()
      { snprintf(buf, N-1, ToCharFmt<T>::Fmt(), a); }
    };

    template<>
    struct ToCharsImpl<bool>
    {
      template<typename Char, std::size_t N>
      static inline void Doit(Char(&buf)[N], bool a) throw()
      { snprintf(buf, N-1, a == true ? "true" : "false" ); }
    };
  } // namespace impl

  struct ToChars
  {
    template<typename T>
    inline char const* operator()(T a) const throw()
    { ::kpq3::impl::ToCharsImpl<T>::Doit(c, a); return c; }

    template<typename T>
    inline char const* Do(T a) const throw()
    { ::kpq3::impl::ToCharsImpl<T>::Doit(c, a); return c; }

  private:
    mutable char c[64];
  };
}

#undef TOCHARS_TRAIT_SPECIAL_DEF
#undef TOCHARS_TRAIT_PARTIAL_DEF

#ifdef snprintf
#undef snprintf
#endif

#endif /* TOCHARS_HH_ */
