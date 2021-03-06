/*
*******************************************************************************
\file env.h
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
\file env.h
\brief Среда выполнения

Модуль содержит описание и реализацию пространства Env,
которая абстрагирует среду выполнения библиотеки GF2.
*******************************************************************************
*/

#ifndef __GF2_ENV
#define __GF2_ENV

#include "gf2/defs.h"
#include "gf2/info.h"

namespace GF2 {

/*!
*******************************************************************************
Класс Env

-#	Печать и трассировка.
-#	Операции с таймером. 
-#	Генерация псевдослучайных чисел.
*******************************************************************************
*/

namespace Env
{
	//! Статическая проверка среды
	int Assert();

	//! Динамическая проверка среды
	int Validate();

	//! Инициализация среды
	int Init();
	
	//! Версия
	const char* Version();

	//! Название
	const char* Name();

	//! Форматированная печать
	void Print(const char* format,...);

	//! Форматированная печать (Unicode)
	void Print(const wchar_t* format,...);

	//! Отладочная печать
	void Trace(const char* format,...);

	//! Время (в ms) с отправного момента в прошлом
	u32 Ticks();

	//! Время (в секундах) с отправного момента в прошлом
	u32 Secs();

	//! Время (в минутах) с отправного момента в прошлом
	u32 Mins();

	//! Генератор псевдослучайных чисел
	u32 Rand();

	//! Инициализировать генератор
	void Seed(u32 seed);

	//! Заполнить буфер псевдослучайными байтами
	void RandMem(void* pMem, size_t size);

	//! Выполнить тест
	bool RunTest(const char* name, bool (*test)());
	bool RunTest(const char* name, bool(*test)(bool), bool verbose = false);

}; // namespace Env

}; // namespace GF2

#endif // __GF2_ENV
