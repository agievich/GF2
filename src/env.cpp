/*
*******************************************************************************
\file env.cpp
\brief The runtime environment
\project GF2 [algebra over GF(2)]
\created 2004.01.01
\version 2020.04.22
\license This program is released under the MIT License. See Copyright Notices
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file env.cpp
\brief Среда выполнения

Модуль содержит реализацию пространства Env, которое абстрагирует среду 
выполнения библиотеки GF2.
*******************************************************************************
*/

#include "gf2/env.h"
#include "gf2/info.h"

using namespace GF2;

#include <assert.h>
#include <locale.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#if defined OS_WIN
	#include <windows.h>
#elif defined OS_LINUX
	#include <sys/types.h>
	#include <sys/time.h>
	#include <unistd.h>
#elif defined OS_APPLE
	#include <mach/clock.h>
	#include <mach/mach.h>
#endif

// Статическая проверка среды
int Env::Assert()
{
	assert(sizeof(size_t) % sizeof(word) == 0);
	return 0;
}

// Динамическая проверка среды
int Env::Validate()
{
	return 0;
}

static char _name[128]; //< имя среды

// Инициализация среды
int Env::Init()
{	
	// получить имя
#if defined OS_WIN
	::STARTUPINFOA si;
	::GetStartupInfoA(&si);
	if (::strlen(si.lpTitle) >= sizeof(_name))
		::strncpy(_name, si.lpTitle, sizeof(_name) - 1);
	else
		::strcpy(_name, si.lpTitle);
#elif defined OS_LINUX
	char filename[256];
	::sprintf(filename, "/proc/%d/cmdline", ::getpid());
	FILE* f = ::fopen(filename, "r");
	if (f) 
	{
	    if (::fgets(_name, sizeof(_name), f) == 0)
			::strcpy(_name, "gf2app");
	    ::fclose(f);
	}
	else
		::strcpy(_name, "gf2app");
#else	
	::strcpy(_name, "gf2app");
#endif	
	return 0;
}

// проверить и настроить среду
static int _check_and_init = 
	Env::Assert() + Env::Validate() + Env::Init();

// Версия
const char* Env::Version()
{	
	return GF2_VERSION;
}

// Название
const char* Env::Name()
{	
	return _name;
}

// Форматированная печать
void Env::Print(const char* format,...)
{	
	va_list args;
	va_start(args, format);
	::vprintf(format, args);
	va_end(args);
}

// Форматированная печать (Unicode)
void Env::Print(const wchar_t* format,...)
{	
	va_list args;
	va_start(args, format);
	::vwprintf(format, args);
	va_end(args);
}

// Отладочная печать
void Env::Trace(const char* format,...)
{	
	static char prevbuf[1024];
	static char buf[1024];
	// пустая форматная строка?
	if (::strlen(format) == 0)
	{
		prevbuf[0] = '\0';
#ifdef OS_WIN
		::SetConsoleTitleA(Name());
#elif defined OS_UNIX
		// clear line from cursor right
		// [http://www.climagic.org/mirrors/VT100_Escape_Codes.html]
		printf("\033[0K");
#endif
		return;
	}
	// сообщение
	va_list args;
	va_start(args, format);
	::vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
#if defined OS_WIN
	::CharToOemA(buf, (char*)buf);
#endif
	if (::strcmp(prevbuf, buf))
	{
#if defined OS_WIN
		::SetConsoleTitleA(buf);
#elif defined OS_UNIX
		// clear line from cursor right
		printf("\033[0K");
		// print in green 
		// [ANSI_COLOR_GREEN = "\x1b[32m", ANSI_COLOR_RESET = "\x1b[0m"]
		printf("\x1b[32m%s\x1b[0m", buf);
		// move cursor backward
		printf("\033[%dD", (int)strlen(buf));
#else
		fprintf(stderr, "%s ", buf);
#endif
		::strcpy(prevbuf, buf);
	}
}

// Время (в ms) с отправного момента в прошлом
u32 Env::Ticks()
{
	u32 ticks = 0;
#if defined OS_WIN
	ticks = (u32)::GetTickCount();
#elif defined OS_LINUX
	timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    ticks = ts.tv_nsec / 1000000;
    ticks += ts.tv_sec * 1000;
#elif defined OS_APPLE
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
    ticks = mts.tv_nsec / 1000000;
    ticks += mts.tv_sec * 1000;
#endif
	return ticks;
}

// Время (в секундах) с отправного момента в прошлом
u32 Env::Secs()
{	
	return Ticks() / 1000;
}

// Время (в минутах) с отправного момента в прошлом
u32 Env::Mins()
{	
	return Ticks() / 60000;
}

static u32 _randx = 0xF8B7BB93;
static u32 _randy = 0xBEE3B54B;
static u32 _randz = 0x1F6B7FBD + Env::Ticks();

// Генератор псевдослучайных чисел (COMBO)
u32 Env::Rand()
{	
	u32 v = _randx * _randy;
   	_randx = _randy; _randy = v;
	_randz = (_randz & 0xFFFF) * 30903 + (_randz >> 16);
	return _randy + _randz;
}

// Посеять генератор
void Env::Seed(u32 seed)
{	
	_randx = 0xF8B7BB93;
	_randy = 0xBEE3B54B;
	_randz = 0x1F6B7FBD + seed;
}

// Заполнить буфер псевдослучайными байтами
void Env::RandMem(void* pMem, size_t size)
{
	if (size == 0) return;
	u32* pMem32 = (u32*)pMem;
	// заполняем все слова, кроме последнего (возможно неполного)
	for (size_t i = 0; i + 1 < size / 4; *(pMem32++) = Rand(), ++i);
	// заполняем последнее слово
	u32 rnd = Rand();
	if (size %= 4)
		::memcpy(pMem32, &rnd, size);
	else
		*pMem32 = rnd;
}

bool Env::RunTest(const char* name, bool(*test)())
{
	Print("%s: ", name);
	bool success = test();
	Print("%s\n", success ? "OK" : "Err");
	return success;
}

bool Env::RunTest(const char* name, bool(*test)(bool), bool verbose)
{
	Print("%s: ", name);
	bool success = test(verbose);
	Print("%s\n", success ? "OK" : "Err");
	return success;
}
