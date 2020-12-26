/*
 * q_typetraits.hh
 *
 *  Created on: 15.01.2011
 *      Author: area
 */
#ifndef Q_TYPETRAITS_HH_
#define Q_TYPETRAITS_HH_
#if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3) && !defined(__GCCXML__)))
#define KPQ3_HAVE_GCC_INTRINSIC
#endif
#include <cstddef>

namespace Kpq3 { namespace Traits
{
  template<typename T>
  struct AlignmentOf;

  namespace Private
  {
    template<typename T>
    struct alignment_of_hack
    {
      char c;
      T t;
      alignment_of_hack();
    };

    template<unsigned A, unsigned S>
    struct alignment_logic
    {
      enum { Value = A < S ? A : S };
    };

    template<typename T>
    struct AlignmentOfImpl
    {
#ifndef KPQ3_HAVE_GCC_INTRINSIC
	  enum { Value = (alignment_logic<sizeof(alignment_of_hack<T>) - sizeof(T), __alignof(T)>::Value));
#else
      enum { Value =__alignof__(T) };
#endif
    };
  } // namespace Private

  template< typename T >
  struct AlignmentOf
  {
	  enum { Value = Private::AlignmentOfImpl<T>::Value };
  };

  template <typename T>
  struct AlignmentOf<T&> : AlignmentOf<T*>
  {};

  template<>
  struct AlignmentOf<void>
  {
    enum { Value = 0 };
  };

  template<>
  struct AlignmentOf<void const>
  {
	    enum { Value = 0 };
  };
  template<>
  struct AlignmentOf<void volatile>
  {
	  enum { Value = 0 };
  };

  template<>
  struct AlignmentOf<void const volatile>
  {
	  enum { Value = 0 };
  };
} // namespace Traits
} // namespace Kpq3


#endif /* Q_TYPETRAITS_HH_ */
