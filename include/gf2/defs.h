/*
*******************************************************************************
\file defs.h
\brief Basic definitions
\project GF2 [GF(2) algebra library]
\author (С) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2004.01.01
\version 2016.07.06
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file defs.h
\brief Базовые определения

Модуль содержит стандартные определения и вспомогательные функции 
библиотеки GF2.

\remark Используются фрагменты файлов определений библиотеки bee2.
*******************************************************************************
*/

#ifndef __GF2_DEFS
#define __GF2_DEFS

#include <limits.h>
#include <stddef.h>

namespace GF2{

/*!
*******************************************************************************
\def OS_WIN
\brief Операционная система линейки Windows

\def OS_UNIX
\brief Операционная система линейки Unix
\remark Включает линейки Linux и MAC OS X

\def OS_LINUX
\brief Операционная система линейки Linux

\def OS_APPLE
\brief Операционная система линеек OS X / iOS
*******************************************************************************
*/

#if defined(_WIN32) || defined(_WIN64) || defined(_WINNT) ||\
	defined(__WIN32__) || defined(__WIN64__)  || defined(__WINNT__)
	#define OS_WIN
	#undef OS_UNIX
	#undef OS_LINUX
	#undef OS_APPLE
#elif defined(unix) || defined(_unix_) || defined(__unix__) ||\
	defined(__APPLE__)
	#undef OS_WIN
	#define OS_UNIX
	#if defined(__unix__)
		#define OS_LINUX
		#undef OS_APPLE
	#elif defined(__APPLE__)
		#undef OS_LINUX
		#define OS_APPLE
	#endif
#else
	#undef OS_WIN
	#undef OS_UNIX
	#undef OS_LINUX
	#undef OS_APPLE
#endif

/*!
*******************************************************************************
\typedef octet
\brief Октет

\typedef u8
\brief 8-разрядное беззнаковое целое

\typedef i8
\brief 8-разрядное знаковое целое

\typedef u16
\brief 16-разрядное беззнаковое целое

\typedef i16
\brief 16-разрядное знаковое целое

\typedef u32
\brief 32-разрядное беззнаковое целое

\typedef i32
\brief 32-разрядное знаковое целое
*******************************************************************************
*/

#undef U8_SUPPORT
#undef U16_SUPPORT
#undef U32_SUPPORT
#undef U64_SUPPORT
#undef U128_SUPPORT

#if (UCHAR_MAX == 255)
	typedef unsigned char u8;
	typedef signed char i8;
	typedef u8 octet;
	#define U8_SUPPORT
#else
	#error "Unsupported char size"
#endif

#if (USHRT_MAX == 65535)
	typedef unsigned short u16;
	typedef signed short i16;
	#define U16_SUPPORT
#else
	#error "Unsupported short size"
#endif

#if (UINT_MAX == 65535u)
	#if (ULONG_MAX == 4294967295ul)
		typedef unsigned long u32;
		typedef signed long i32;
		#define U32_SUPPORT
	#else
		#error "Unsupported long size"
	#endif
#elif (UINT_MAX == 4294967295u)
	typedef unsigned int u32;
	typedef signed int i32;
	#define U32_SUPPORT
	#if (ULONG_MAX == 4294967295ul)
		#if !defined(ULLONG_MAX) || (ULLONG_MAX == 4294967295ull)
			#error "Unsupported int/long/long long configuration"
		#elif (ULLONG_MAX == 18446744073709551615ull)
			typedef unsigned long long u64;
			typedef signed long long i64;
			#define U64_SUPPORT
		#else
			#error "Unsupported int/long/long long configuration"
		#endif
	#elif (ULONG_MAX == 18446744073709551615ul)
		typedef unsigned long u64;
		typedef signed long i64;
		#define U64_SUPPORT
		#if defined(__GNUC__) && (__WORDSIZE == 64)
			typedef __int128 i128;
			typedef unsigned __int128 u128;
			#define U128_SUPPORT
		#endif
	#else
		#error "Unsupported int/long configuration"
	#endif
#elif (UINT_MAX == 18446744073709551615u)
	#if (ULONG_MAX == 18446744073709551615ul)
		#if !defined(ULLONG_MAX) || (ULLONG_MAX == 18446744073709551615ull)
			#error "Unsupported int/long/long long configuration"
		#elif (ULLONG_MAX == 340282366920938463463374607431768211455ull)
			typedef unsigned long long u128;
			typedef signed long long i128;
			#define U128_SUPPORT
		#else
			#error "Unsupported int/long/long long configuration"
		#endif
	#elif (ULONG_MAX == 340282366920938463463374607431768211455ul)
		typedef unsigned long u128;
		typedef signed long i128;
		#define U128_SUPPORT
	#else
		#error "Unsupported long size"
	#endif
#else
	#error "Unsupported int size"
#endif

#if !defined(U8_SUPPORT) || !defined(U16_SUPPORT) || !defined(U32_SUPPORT)
	#error "One of the base types is not supported"
#endif

/*!
*******************************************************************************
\def B_PER_W
\brief Число битов в машинном слове

\def O_PER_W
\brief Число октетов в машинном слове

\def B_PER_S
\brief Число битов в size_t

\def O_PER_S
\brief Число октетов в size_t

\typedef word
\brief Машинное слово

\typedef dword
\brief Двойное машинное слово
*******************************************************************************
*/

#if defined(__WORDSIZE)
	#if (__WORDSIZE == 16)
		#define B_PER_W 16
		typedef u16 word;
		typedef u32 dword;
	#elif (__WORDSIZE == 32)
		#define B_PER_W 32
		typedef u32 word;
		typedef u64 dword;
	#elif (__WORDSIZE == 64)
		#define B_PER_W 64
		typedef u64 word;
		typedef u128 dword;
	#else
		#error "Unsupported word size"
	#endif
#else
	#if (UINT_MAX == 65535u)
		#define B_PER_W 16
		typedef u16 word;
		typedef u32 dword;
	#elif (UINT_MAX == 4294967295u)
		#define B_PER_W 32
		typedef u32 word;
		typedef u64 dword;
	#elif (UINT_MAX == 18446744073709551615u)
		#define B_PER_W 64
		typedef u64 word;
		typedef u128 dword;
	#else
		#error "Unsupported word size"
	#endif
#endif

#if (B_PER_W != 16 && B_PER_W != 32 && B_PER_W != 64)
	#error "Unsupported word size"
#endif

#define O_PER_W (B_PER_W / 8)
#define O_PER_S (B_PER_S / 8)

#define SIZE_0 ((size_t)0)
#define SIZE_1 ((size_t)1)
#ifndef SIZE_MAX
	#ifdef __SIZE_MAX__
		#define SIZE_MAX __SIZE_MAX__
	#else
		#error "Can't determine SIZE_MAX"
	#endif
#endif

#if (SIZE_MAX == 65535u)
	#define B_PER_S 16
#elif (SIZE_MAX == 4294967295u)
	#define B_PER_S 32
#elif (SIZE_MAX == 18446744073709551615u)
	#define B_PER_S 64
#else
	#error "Unsupported size_t size"
#endif

/*!
*******************************************************************************
\def MIN2
\brief Минимум из двух элементов

\def MIN3
\brief Минимум из трех элементов

\def MAX2
\brief Максимум из двух элементов

\def MAX3
\brief Максимум из трех элементов
*******************************************************************************
*/

#define MIN2(a, b) ((a) < (b) ? (a) : (b))
#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN3(a, b, c) MIN2(a, MIN2(b, c))
#define MAX3(a, b, c) MAX2(a, MAX2(b, c))

};

#endif //__GF2_DEFS
