/**********************************************************************************************************************
*	$Id$
**********************************************************************************************************************/
/*!	@file
*	@brief
*	Unified type definitions.
**********************************************************************************************************************/
#ifndef	CTYPES
#define	CTYPES						"          CTYPES >>>    "

#ifndef	FILE__
	#define	FILE__	__FILE__
						#endif
#ifdef	PreInc_h__
	#include	PreInc_h__
						#endif
#ifdef	__cplusplus
	extern	"C"
	{
#endif



/**//*!	@defgroup _data_types
	*	Elementary data types.
	******************************************************/
/*!	@{
 */
	#ifndef	UNSG8
	#define	UNSG8	UNSG8_
						#endif
	#ifndef	SIGN8
	#define	SIGN8	SIGN8_
						#endif
	#ifndef	UNSG16
	#define	UNSG16	UNSG16_
						#endif
	#ifndef	SIGN16
	#define	SIGN16	SIGN16_
						#endif
	#ifndef	UNSG32
	#define	UNSG32	UNSG32_
						#endif
	#ifndef	SIGN32
	#define	SIGN32	SIGN32_
						#endif
	#ifndef	UNSG64
	#define	UNSG64	UNSG64_
						#endif
	#ifndef	SIGN64
	#define	SIGN64	SIGN64_
						#endif
	#ifndef	REAL32
	#define	REAL32	REAL32_
						#endif
	#ifndef	REAL64
	#define	REAL64	REAL64_
						#endif

	//*	Microsoft Visual C++
	#if		defined(_MSC_VER)
    typedef	unsigned char		UNSG8_;
    typedef	signed char			SIGN8_;
    typedef	unsigned short		UNSG16_;
    typedef	signed short		SIGN16_;
    typedef	unsigned int		UNSG32_;
    typedef	signed int			SIGN32_;
    typedef	unsigned __int64	UNSG64_;
    typedef	signed __int64		SIGN64_;
    typedef	float				REAL32_;
    typedef	double				REAL64_;

	//*	GNU gcc/g++
	#elif	defined(__GNUC__)
    typedef	unsigned char		UNSG8_;
    typedef	signed char			SIGN8_;
    typedef	unsigned short		UNSG16_;
    typedef	signed short		SIGN16_;
    typedef	unsigned int		UNSG32_;
    typedef	signed int			SIGN32_;
    typedef	unsigned long long	UNSG64_;
    typedef	signed long long	SIGN64_;
    typedef	float				REAL32_;
    typedef	double				REAL64_;

	//*	ARMCC (RVCT)
	#elif	defined(__ARMCC_VERSION)
    typedef	unsigned char		UNSG8_;
    typedef	signed char			SIGN8_;
    typedef	unsigned short		UNSG16_;
    typedef	signed short		SIGN16_;
    typedef	unsigned int		UNSG32_;
    typedef	signed int			SIGN32_;
    typedef	unsigned long long	UNSG64_;
    typedef	signed long long	SIGN64_;
    typedef	float				REAL32_;
    typedef	double				REAL64_;
	#endif

/*!	@}
 */



/**//*!	@defgroup _more_types
	*	More types/symbols may appear in API definitions.
	******************************************************/
/*!	@{
 */
	#ifndef	Txxb
	#define	Txxb
	typedef	UNSG8				T8b;
	typedef	UNSG16				T16b;
	typedef	UNSG32				T32b;
	typedef	UNSG32				T64b  [2];
	typedef	UNSG32				T96b  [3];
	typedef	UNSG32				T128b [4];
	typedef	UNSG32				T160b [5];
	typedef	UNSG32				T192b [6];
	typedef	UNSG32				T224b [7];
	typedef	UNSG32				T256b [8];
									#endif
/*!	@}
 */



#ifdef	__cplusplus
	}
#endif

#endif
/**********************************************************************************************************************
ENDOFFILE
*/

