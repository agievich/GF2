/*
*******************************************************************************
\file env.cpp
\brief The runtime environment
\project GF2 [GF(2) algebra library]
\author (С) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2004.01.01
\version 2016.07.05
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
#ifdef OS_WIN
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
#ifdef OS_WIN
	// проверка версии Windows
	assert((::GetVersion() & 0x8000) == 0);
	// проверка размерностей
	assert(sizeof(size_t) % sizeof(word) == 0);
#endif
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
#ifdef OS_WIN
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
	// пустая форматная строка?
	if (::strlen(format) == 0)
	{
#ifdef OS_WIN
		::SetConsoleTitleA(Name());
#endif
		return;
	}
	// сообщение
	static char buffer[1024];
	va_list args;
	va_start(args, format);
	::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
#ifdef OS_WIN
	// меняем кодировку
	::CharToOemA(buffer, (char*)buffer);
	// сравниваем с предыдущим названием
	static char prevbuffer[sizeof(buffer)];
	::GetConsoleTitleA(prevbuffer, sizeof(prevbuffer));
	if (::strcmp(prevbuffer, buffer) == 0)
		return;
	::SetConsoleTitleA(buffer);
#else
	fprintf(stderr, "%s\n", buffer);
#endif
}

// Время (в ms) с отправного момента в прошлом
u32 Env::Ticks()
{	
#ifdef OS_WIN
	return (u32)::GetTickCount();
#elif defined OS_LINUX
	timespec ts;
    u32 ticks = 0;
    ::clock_gettime( CLOCK_MONOTONIC, &ts);
    ticks = ts.tv_nsec / 1000000;
    ticks += ts.tv_sec * 1000;
    return ticks;
#elif defined OS_APPLE
	clock_serv_t cclock;
	mach_timespec_t mts;
    u32 ticks = 0;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
    ticks = mts.tv_nsec / 1000000;
    ticks += mts.tv_sec * 1000;
	return ticks;
#endif
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
void Env::RandMem(void* pMem, word size)
{
	if (size == 0) return;
	u32* pMem32 = (u32*)pMem;
	// заполняем все слова, кроме последнего (возможно неполного)
	for (word i = 0; i + 1 < size / 4; *(pMem32++) = Rand(), i++);
	// заполняем последнее слово
	u32 rnd = Rand();
	if (size %= 4)
		::memcpy(pMem32, &rnd, size);
	else
		*pMem32 = rnd;
}
