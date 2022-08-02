
//types.h//

//Define to prevent recursive inclusion
#ifndef __TYPES_H
#define __TYPES_H

/*
	#define int8_t    Int8
	#define uint8_t   Uint8
	#define int16_t   Int16
	#define uint16_t  Uint16
	#define int32_t   Int32
	#define uint32_t  Uint32
*/

	#define int8_t			signed char
	#define uint8_t			unsigned char
	#define int16_t			short
	#define uint16_t		unsigned short
	#define int32_t			int
	#define uint32_t		unsigned int
	#define int64_t			long long
	#define uint64_t		unsigned long long
	#define float32			float

/*
	typedef signed char			int8_t;
	typedef unsigned char		uint8_t;
	typedef short				int16_t;
	typedef unsigned short		uint16_t;
	typedef int					int32_t;
	typedef unsigned int		uint32_t;
	typedef unsigned			uint32_t;
	typedef long long			int64_t;
	typedef unsigned long long	uint64_t;
*/
	
#define SYSTEM_POINTER_SZ		unsigned
#define SIZE_T					SYSTEM_POINTER_SZ

#define ubase_t				uint32_t //на случай если нужно все переменные не критиные к размеру заменить на другой тип
#define base_t				int32_t //на случай если нужно все переменные не критиные к размеру заменить на другой тип

#if defined (__ICCARM__) || defined (__ATOLLIC__)      // IAR Compiler
#define NULL					((void *)0)
#endif//defined (__ICCARM__) || defined (__ATOLLIC__)

#endif // __TYPES_H
