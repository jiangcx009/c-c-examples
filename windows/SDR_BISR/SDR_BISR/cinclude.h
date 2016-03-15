/**********************************************************************************************************************
*	$Id$
**********************************************************************************************************************/
/*!	@file
*	@brief
*	Recommended to include by all C/C++ files.
*
*	Pre-definition build options:
*	-	{__CODE_LINK__}				Pre-define for embedded porting:
*									-	not defined: for non-embedded code, most standard libraries will be included
*									-	=0: recommened for deeply embedded porting
*									-	=1: will include (user customized) "string.h"
*									-	=2: will include (user customized) "string.h" & "malloc.h"
*	-	{__big_endian__}			Pre-define big-endian processors.
**********************************************************************************************************************/
#ifndef	CINCLUDE
#define	CINCLUDE					"        CINCLUDE >>>    "



/**//*!	@defgroup _includes
	*	Standard includes for non-embedded use.
	******************************************************/
/*!	@{
 */
#ifndef	__CODE_LINK__
	#ifndef	_CRT_SECURE_NO_DEPRECATE
	#define	_CRT_SECURE_NO_DEPRECATE
	#endif
	#include	<stdio.h>
	#include	<stdlib.h>
	#include	<stdarg.h>
	#include	<string.h>
	#include	<math.h>
	#include	<time.h>
	#include	<assert.h>
	#include	<errno.h>
#elif	__CODE_LINK__ > 0
	#include	"string.h"			//*	customized: memcpy, memset, and etc.
	#if	__CODE_LINK__ > 1
	#include	"malloc.h"			//*	customized: malloc, free, and etc.
	#endif
#endif

	//*	Microsoft Visual C++
	#if		defined(_MSC_VER)
	#include	<io.h>
	#include	<direct.h>
	#include	<windows.h>

	/*!	@def	ptr32u
	*	Cast a pointer to a 32b unsigned integer (unsafe for 64b compiler).
	*	@param[in]		ptr		any pointer
	*	@return			UNSG32						:	32b integer value of the pointer
	******************************************************************************************************************/
	#define	ptr32u(ptr)				((UNSG32)PtrToUlong((char*)ptr))

	//*	GNU gcc/g++
	#elif	defined(__GNUC__)
	#include	<unistd.h>
	#include	<sys/time.h>
	#include	<sys/types.h>
	#define	ptr32u(ptr)				((UNSG32)(ptr))
	#define	_stricmp				strcasecmp
	#define	_unlink					unlink
	#define	_open					open
	#define	_close					close
	#define	_read					read
	#define	_write					write
	#define	_tell					tell
	#define	_lseek					lseek
	#define	_lseeki64				lseeki64

	//*	ARMCC (RVCT)
	#elif	defined(__ARMCC_VERSION)
    #define	ptr32u(ptr)				((UNSG32)(ptr))
    #define	_stricmp				strcmp
    #define	_unlink					remove	
	#endif

	#include	"ctypes.h"
	#include	"cmacros.h"

/*!	@}
 */



/**//*!	@defgroup _more_defines
	*	Note: function return convention:
	*	-	0: success
	*	-	<0: error
	*	-	>0: success with a positive return value
	******************************************************/
/*!	@{
 */
	#ifndef	SUCCESS
	#define	SUCCESS					0					//*	for function return
	#define	ERR_UNKNOWN				-1					//*	error type not defined
	#define	ERR_POINTER				-2					//*	invalid pointer
	#define	ERR_FILE				-3					//*	file access failure
	#define	ERR_FIFO				-4					//*	fifo overflow or underflow
	#define	ERR_MEMORY				-5					//*	not enough memory or allocation failure
	#define	ERR_MISMATCH			-6					//*	mis-matches found in hand-shaking
	#define	ERR_PARAMETER			-7					//*	false function parameter
	#define	ERR_HARDWARE			-8					//*	hardware error
	#define	ERR_TIMING				-9					//*	sychronize-related violation
														#endif
	#ifndef	Global
		#ifdef	__MAIN__
		#define	Global
		#define	GlobalAssign(g, x)	g = (x)

		#else
		#define	Global				extern
		#define	GlobalAssign(g, x)	g
		#endif
	#endif

	#ifdef	__CODE_LINK__
	#define	NonPortable				INLINE
	#define	NonPortableReturn(r)	{ return r; }
	#else

	/*!	@def	NonPortable
	*	Prefix for a non-portable function declaration to avoit porting troubles.
	******************************************************************************************************************/
	#define	NonPortable

	/*!	@def	NonPortableReturn
	*	Suffix for a non-portable function declaration to avoit porting troubles.
	*	@param[in]		r		default return value at embedded porting
	******************************************************************************************************************/
	#define	NonPortableReturn(r)
	#endif

/**	ENDOFSECTION
 */



#endif
/**********************************************************************************************************************
ENDOFFILE
*/

