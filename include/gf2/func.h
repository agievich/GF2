/*
*******************************************************************************
\file func.h
\brief Functions {0, 1}^n \to T
\project GF2 [algebra over GF(2)]
\created 2004.06.10
\version 2020.04.10
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file func.h

\brief Функции {0,1}^n \to T

Модуль содержит описание и реализацию классов, поддерживающего манипуляции 
с функциями {0,1}^n \to T для различных типов T.

\todo Разобраться с противоречием: размеры массивов задаются типом size_t,
а индексируются типом word.
*******************************************************************************
*/

#ifndef __GF2_FUNC
#define __GF2_FUNC

#include "gf2/env.h"
#include "gf2/ideal.h"
#include "gf2/word.h"
#include "gf2/zz.h"
#include <iostream>

namespace GF2 {

/*!
*******************************************************************************
\file func.h

\section func-intro Общее описание

Модуль содержит описание и реализацию классов, поддерживающих манипуляции 
с функциями от n булевых переменных и значениями в некоторой 
алгебраической структуре T.

\section func-base Базовый класс Func

-#	Поддерживает манипуляции с функциями от n булевых переменных и значениями 
	в некоторой алгебраической структуре T.
-#	Число переменных n должно быть меньше числа битов в word.
-#	Класс T должен допускать создание нулевых экземпляров Т(0), 
	сравнения, присваивания и вывод в поток.
-#	Для индексации значений функции используются числа от 0 до 
	Size() - 1. Экземпляры Word<n> неявно приводятся к таким числам.
*******************************************************************************
*/

template<size_t _n, class _T> class Func
{
// прообразы
private:
	enum {__assert = 1 / word(_n < sizeof(word) * 8)}; //< проверка
public:
	enum {n = _n}; //< раскрытие числа переменных
	typedef Word<_n> Preimage; //< тип прообразов

// образы
public:
	typedef _T Image; //< тип образов
protected:
	enum {_size = size_t(1) << _n}; //< число образов
private:
	_T _vals[_size]; //< массив образов

// базовые операции
public:
	//! Число образов
	/*! Определяется число образов. */
	static size_t Size()
	{	
		return _size;
	}

	//! Установить значение
	/*! Установить значение от x равным val. */
	void Set(word x, const _T& val)
	{	
		assert(x < _size);
		_vals[x] = val;
	}

	//! Значение
	/*! Возвратить значение от x. */
	_T Get(word x) const
	{	
		assert(x < _size);
		return _vals[x];
	}

	//! Значение
	/*! Возвратить ссылку на значение от x. */
	_T& Get(word x)
	{	
		assert(x < _size);
		return _vals[x];
	}

	//! Количество значений
	/*! Определяется количество значений valRight среди образов. */
	size_t Count(const _T& valRight) const
	{	
		size_t count = 0;
		for (word x = 0; x < _size; x++)
			if (_vals[x] == valRight) count++;
		return count;
	}

	//! Максимум
	/*! Определяется максимальное значение. */
	const _T& Max() const
	{	
		word xmax = 0;
		for (word x = 1; x < _size; x++)
			if (_vals[x] > _vals[xmax]) xmax = x;
		return _vals[xmax];
	}

	//! Минимум
	/*! Определяется минимальное значение. */
	const _T& Min() const
	{	
		word xmin = 0;
		for (word x = 1; x < _size; x++)
			if (_vals[x] < _vals[xmin]) xmin = x;
		return _vals[xmin];
	}

// операторы
public:
	//! Значение
	/*! Возвратить значение от x. */
	_T operator[](word x) const
	{	
		assert(x < _size);
		return _vals[x];
	}

	//! Значение
	/*! Возвратить ссылку на значение от x. */
	_T& operator[](word x)
	{	
		assert(x < _size);
		return _vals[x];
	}

	//! Значение
	/*! Возвратить значение от x. */
	_T operator()(word x) const
	{
		assert(x < _size);
		return _vals[x];
	}

	//! Значение
	/*! Возвратить ссылку на значение от x. */
	_T& operator()(word x)
	{	
		assert(x < _size);
		return _vals[x];
	}

	//! Присваивание
	/*! Присваивание всем образам значения valRight. */
	Func& operator=(const _T& valRight)
	{	
		for (word x = 0; x < _size; x++)
			_vals[x] = valRight;
		return *this;
	}

	//! Присваивание
	/*! Присваивание образам значений из массива valsRight. */
	Func& operator=(const _T (&valsRight)[_size])
	{	
		for (word x = 0; x < _size; x++)
			_vals[x] = valsRight[x];
		return *this;
	}

	//! Присваивание
	/*! Присваивание значения-функции fRight. */
	Func& operator=(const Func& fRight)
	{	
		if (&fRight != this)
			for (word x = 0; x < _size; x++)
				_vals[x] = fRight.Get(x);
		return *this;
	}

	//! Равенство
	/*! Проверяется равенство функции fRight. */
	bool operator==(const Func& fRight) const
	{
		for (word x = 0; x < _size; x++)
			if (_vals[x] != fRight.Get(x))
				return false;
		return true;
	}

	//! Неравенство
	/*! Проверяется неравенство функции fRight. */
	bool operator!=(const Func& fRight) const
	{
		return !operator==(fRight);
	}

	//! Меньше?
	/*! Проверяется, что функция лексикографические меньше fRight. */
	bool operator<(const Func& fRight) const
	{
		for (word x = 0; x < _size; x++)
			if (_vals[x] < fRight.Get(x))
				return true;
			else if (_vals[x] > fRight.Get(x))
				return false;
		return false;
	}

	//! Не больше?
	/*! Проверяется, что функция лексикографические не больше fRight. */
	bool operator<=(const Func& fRight) const
	{	
		for (word x = 0; x < _size; x++)
			if (_vals[x] < fRight.Get(x))
				return true;
			else if (_vals[x] > fRight.Get(x))
				return false;
		return true;
	}

	//! Больше?
	/*! Проверяется, что функция лексикографические больше fRight. */
	bool operator>(const Func& fRight) const
	{
		return !operator<=(fRight);
	}

	//! Не меньше?
	/*! Проверяется, что функция лексикографические не менььше fRight. */
	bool operator>=(const Func& fRight) const
	{
		return !operator<(fRight);
	}

// конструкторы
public:
	//! Конструктор по умолчанию
	/*! Создается функция с одинаковыми значениями valRight 
		(нулевыми по умолчанию). */
	Func(const _T& valRight = _T(0))
	{	
		for (word x = 0; x < _size; x++)
			_vals[x] = valRight;
	}

	//! Конструктор по значениям
	/*! Создается функция со значениями из массива valsRight. */
	Func(const _T valsRight[_size])
	{
		for (word x = 0; x < _size; x++)
			_vals[x] = valsRight[x];
	}

	//! Конструктор копирования
	/*! Создается копия функции fRight. */
	Func(const Func& fRight)
	{
		for (word x = 0; x < _size; x++)
			Set(x, fRight.Get(x));
	}
};

//! Вывод в поток
/*! Функция fRight выводится в поток os. */
template<class _Char, class _Traits, size_t _n, class _T> inline 
std::basic_ostream<_Char, _Traits>& 
operator<<(std::basic_ostream<_Char, _Traits>& os, const Func<_n, _T>& fRight)
{
	Word<_n> x;
	do
	{
		if (!x.IsAllZero()) os << ' ';
		os << fRight(x);
	}
	while (x.Next());
	return os;
}

//! Ввод из потока
/*! Функция fRight вводится из потока is. */
template<class _Char, class _Traits, size_t _n, class _T> inline 
std::basic_istream<_Char, _Traits>& 
operator>>(std::basic_istream<_Char, _Traits>& is, Func<_n, _T>& fRight)
{
	Word<_n> x;
	do
	{
		typename Func<_n, _T>::Image y;
		is >> y;
		// ошибка чтения?
		if (is.fail() || is.bad())
			break;
		// все нормально
		fRight.Set(x, y);
	}
	while (x.Next());
	return is;
}

/*!
*******************************************************************************
Класс BFunc

-#	Поддерживает манипуляции с булевыми функциями от n переменных.
-#	Методы From() / To() поддерживают представления булевых функций 
	многочленами Жегалкина и коэффициентами Уолша -- Адамара.

В методе FWHT() реализовано быстрое преобразование Уолша-Адамара. За основу 
взята реализация http://www.musicdsp.org/showone.php?id=18. В неявной матрице 
Уолша--Адамара используется так называемая последовательная нумерация строк 
(sequence ordering). Порядок нумерации отличаются от обычного 
(natural ordering), реализованного в методе To(), что приводит 
к перестановке компонентов спектра Уолша-Адамара и инверсии знаков. При этом,
впрочем, максимальное абсолютное значение спектра, которое используется 
в методе Nl(), не меняется.
*******************************************************************************
*/

template<size_t _n> class BFunc : public Func<_n, bool>
{
public:
	using typename Func<_n, bool>::Image;
	using typename Func<_n, bool>::Preimage;
protected:
	using Func<_n, bool>::_size;
	static unsigned abs(int x) { return unsigned(x < 0 ? -x : x); }
public:
	using Func<_n, bool>::Get;
	using Func<_n, bool>::Set;
	using Func<_n, bool>::Size;
// операции
public:
	//! Построение многочлена по функции
	/*! По булевой функции строится многочлен Жегалкина polyRight. */
	template<class _O>
	void To(MPoly<_n, _O>& polyRight) const
	{	
		polyRight = 0;
		// цикл по мономам
		Monom<_n> mon;
		do
		{
			bool bCoeff = 0;
			Monom<_n> x;
			do bCoeff ^= x.Calc(mon) & Get(x);
			while (x.Next());
			if (bCoeff)
				polyRight += mon;
		}
		while(mon.Next());
	}

	//! Построение многочлена по функции
	/*! По многочлену Жегалкина polyRight определяется булева функция. */
	void From(const MPoly<_n>& polyRight)
	{	
		Preimage x;
		do Set(x, polyRight.Calc(x));			
		while (x.Next());
	}

	//! Преобразование Уолша -- Адамара
	/*! По булевой функции расчитываются и сохраняются в zfRight коэффициенты 
		Уолша -- Адамара. 
	*/
	void To(Func<_n, int>& zfRight) const
	{	
		Preimage x, u;
		do
		{
			zfRight[u] = 0;
			do
				if (Get(x) == (x & u).Parity())
					zfRight[u]++;
				else
					zfRight[u]--;
			while (x.Next());
		}
		while (u.Next());
	}

	//! Обратное преобразование Уолша -- Адамара
	/*! По сохраненным в zfRight коэффициентам Уолша -- Адамара
		восстанавливается булева функция. */
	void From(const Func<_n, int>& zfRight)
	{	
		Preimage u, x;
		do
		{
			int nSum = 0;
			do
				if ((x & u).Parity())
					nSum -= zfRight[u];
				else
					nSum += zfRight[u];
			while (u.Next());
			assert(abs(nSum) == _size);
			Set(x, nSum > 0 ? 0 : 1);
		}
		while (x.Next());
	}

	//! Быстрое преобразование Уолша -- Адамара
	/*! По булевой функции расчитываются и сохраняются в zfRight коэффициенты 
		Уолша -- Адамара. */
	void FWHT(Func<_n, int>& zfRight) const
	{	
		Preimage x;
		// zfRight <- (-1)^this
		do 
			zfRight[x] = Get(x) ? 1 : -1;
		while (x.Next());
		// zf <- FWHT(zf)
		for (word i = 0; i < _n; ++i)
		for (word j = 0; j < _size; j += WORD_1 << (i + 1))
		for (word k = 0; k < WORD_1 << i; ++k)
		{
			int t = zfRight[j + k];
			zfRight[j + k] += zfRight[j + k + (WORD_1 << i)];
			zfRight[j + k + (WORD_1 << i)] = 
				t - zfRight[j + k + (WORD_1 << i)];
		}
	}

	//! Задать наудачу
	/*! Генерация случайной функции. */
	BFunc& Rand()
	{
		Word<_size> w;
		w.Rand();
		for (word x = 0; x < _size; x++)
			Set(x, w[x]);
		return *this;
	}

// характеристики
public:
	//! Степень
	/*! Определяется степень многочлена Жегалкина. */
	int Deg() const
	{	
		MPoly<_n> poly;
		To(poly);
		return poly.Deg();
	}

	//! Максимальный коэфициент Уолша -- Адамара
	/*! Определяется максимальный по модулю коэффициент Уолша -- Адамара. */
	size_t MaxWH() const
	{	
		Func<_n, int> zf;
		FWHT(zf);
		size_t max = 0;
		for (word x = 0; x < _size; x++)
		{
			size_t cur = abs(zf.Get(x));
			if (cur > max) 
				max = cur;
		}
		return max;
	}

	//! Нелинейность
	/*! Определяется нелинейность (расстояние до множества аффинных функций). */
	size_t Nl() const
	{	
		return (_size - MaxWH()) / 2;
	}

	//!	Сбалансированная функция?
	/*!	Проверяется, что функция принимает значения 0 и 1 одинаковое число 
		раз. */
	bool IsBalanced() const
	{	
		size_t ones = 0;
		for (word x = 0; x < _size; ++x)
			if (Get(x))
				++ones;
		return ones * 2 == _size;
	}

	//!	Платовидная функция порядка r?
	/*!	Проверяется, что функция является платовидной порядка r,
		т.е. ненулевые модули коэффициентов Уолша -- Адамара равняются 
		2^{(n - r / 2)}. */
	bool IsPlateaued(word r) const
	{	
		if (r % 2) return false;
		r = WORD_1 << (_n - r / 2);
		Func<_n, int> zf;
		FWHT(zf);
		Word<_n> x;
		do
			if (zf.Get(x) != 0 && abs(zf.Get(x)) != r)
				return false;
		while (x.Next());
		return true;
	}

	//!	Бент-функция?
	/*!	Проверяется, что функция является бент-функцией,
		т.е. модули всех коэффициентов Уолша -- Адамара совпадают. */
	bool IsBent() const
	{
		if (_n % 2) 
			return false;
		Func<_n, int> zf;
		FWHT(zf);
		Word<_n> x;
		do
			if (abs(zf.Get(x)) != (WORD_1 << _n / 2))
				return false;
		while (x.Next());
		return true;
	}

	//! Дуальная бент-функция
	/*!	Бент-функция заменяется дуальной к себе. */
	BFunc<_n>& Dual()
	{
		assert(IsBent());
		Func<_n, int> zf;
		To(zf);
		Word<_n> x;
		do 
			Set(x, zf(x) < 0);
		while (x.Next());
		return *this;
	}

	//!	Влияние единичных ошибок
	/*! Определяется минимальное отклонение от среднего числа случаев, 
		когда изменение некоторой переменной изменит значение функции. */
	size_t PC1() const
	{
		size_t record = 0;
		for (size_t i = 0; i < _n; ++i)
		{
			word mask = WORD_1 << i;
			size_t count = 0;
			for (word x = 0; x < Size(); ++x)
			{
				if (Get(x) != Get(x ^ mask))
					++count;
			}
			if (count > Size() / 2)
				count -= Size() / 2;
			else 
				count = Size() / 2 - count;
			if (count > record)
				record = count;
		}
		return record;
	}

// конструкторы
public:
	//! Конструктор по умолчанию
	/*! Создается функция с одинаковыми значениями valRight 
		(нулевыми по умолчанию). */
	BFunc(bool valRight = 0) : Func<_n, bool>(valRight) {}

	//! Конструктор по значениям из массива
	/*! Создается функция со значениями из массива valsRight. */
	BFunc(const bool valsRight[_size]) : Func<_n, bool>(valsRight) {}

	//! Конструктор по значениям из слова
	/*! Создается функция со значениями из массива valsRight. */
	BFunc(const Word<1 << _n> valsRight)
	{
		for (word x = 0; x < _size; x++)
			Set(x, valsRight[x]);
	}

	//! Конструктор копирования
	/*! Создается копия функции bfRight. */
	BFunc(const BFunc& bfRight) : Func<_n, bool>(bfRight) {}
};

/*******************************************************************************
Класс VFunc

Поддерживает манипуляции с вектор-функциями \{0,1\}^n \to \{0,1\}^m.
*******************************************************************************/

template<size_t _n, size_t _m> class VFunc : public Func<_n, Word<_m> >
{
public:
	using typename Func<_n, Word<_m> >::Image;
	using typename Func<_n, Word<_m> >::Preimage;
protected:
	using Func<_n, Word<_m> >::_size;
public:
	using Func<_n, Word<_m> >::Get;
	using Func<_n, Word<_m> >::Set;
	using Func<_n, Word<_m> >::Size;
// размерность образов
public:
	enum {m = _m}; //< размерность образов

// операции
public:
	//! Установка координатной функции
	/*! Координатная функция в позиции pos устанавливается 
		равной bfRight. */
	void SetCoord(size_t pos, const BFunc<_n>& bfRight)
	{	
		assert(pos < _m);
		for (word x = 0; x < _size; x++)
			Get(x)[pos] = bfRight(x);
	}

	//! Возврат координатной функции
	/*! По ссылке bfRight возвращается координатная функция 
		в позиции pos. */
	void GetCoord(size_t pos, BFunc<_n>& bfRight) const
	{	
		assert(pos < _m);
		for (word x = 0; x < _size; x++)
			bfRight(x) = Get(x)[pos];
	}

	//! Возврат линейной комбинации координатных функции
	/*! По ссылке bfRight возвращается линейная комбинация 
		координатных функций. Коэффициенты линейной комбинации
		задает слово wComb. */
	void GetCoordComb(const Image& wComb, BFunc<_n>& bfRight) const
	{	
		for (word x = 0; x < _size; x++)
			bfRight(x) = (Get(x) & wComb).Parity();
	}

	//! Преобразование в систему многочленов
	/*! Создается система многочленов ideal, которая описывает 
		действие функции. */
	template<class _O>
	void To(Ideal<_n + _m, _O>& ideal) const
	{
		// очистить систему
		ideal.SetEmpty();
		// подготовить переменные
		BFunc<_n> bf;
		MPoly<_n, _O> poly;
		// цикл по координатным функциям
		for (size_t i = 0; i < _m; i++)
		{
			GetCoord(i, bf);
			bf.To(poly);
			// добавляем многочлен y_i - bf(x)
			ideal.Insert(MPoly<_n + _m, _O>(poly) += 
				Monom<_n + _m>(_n + i));
		}
	}

	//! Задать наудачу
	/*! Генерация случайной функции. */
	VFunc& Rand()
	{
		for (word x = 0; x < _size; x++)
			Get(x).Rand();
		return *this;
	}

// характеристики
public:
	//!	Степень
	/*! Определяется максимальная степень координатных функций. */
	int Deg() const
	{	
		BFunc<_n> bf;
		int record = -1, deg;
		for (size_t pos = 0; pos < _m; pos++)
		{
			GetCoord(pos, bf);
			if ((deg = bf.Deg()) > record)
				record = deg;
		}
		return record;
	}

	//!	Полная степень 
	/*! Определяется минимальная степень нунелевых линейных комбинаций 
		координатных функций. */
	int DegSpan() const
	{	
		BFunc<_n> bf;
		int record = _n, deg;
		Image wComb;
		while (wComb.Next())
		{
			GetCoordComb(wComb, bf);
			if ((deg = bf.Deg()) < record)
				record = deg;
		}
		return record;
	}

	//!	Разреженность 
	/*! Определяется минимальное число мономов в многочленах Жегалкина 
		линейных комбинаций координатных функций. */
	size_t Spr() const
	{	
		BFunc<_n> bf;
		MPoly<_n> poly;
		size_t record = Size(), spr;
		Image wComb;
		while (wComb.Next())
		{
			GetCoordComb(wComb, bf);
			bf.To(poly);
			if ((spr = poly.Size()) < record)
				record = spr;
		}
		return record;
	}

	//!	Нелинейность
	/*! Определяется минимальная нелинейность 
		невырожденных линейных комбинаций координатных функций. */
	size_t Nl() const
	{
		BFunc<_n> bf;
		size_t record = SIZE_MAX, nl;
		Image wComb;
		while (wComb.Next())
		{
			GetCoordComb(wComb, bf);
			if ((nl = bf.Nl()) < record)
				record = nl;
		}
		return record;
	}

	//!	Разностная характеристика
	/*! Определяется разностная характеристика одного из четырех типов:
		- 0	Xor -> Xor
		- 1	Xor -> Add
		- 2	Add -> Xor
		- 3	Add -> Add
	*/
	size_t Dc(int type) const
	{
		Func<_m, size_t> count;
		size_t record = 0, dc;
		word x;
		Preimage alpha;
		while (alpha.Next())
		{
			count = size_t(0);
			switch (type)
			{
			case 0:
				for (x = 0; x < _size; x++) 
					count[Get(x ^ alpha) ^= Get(x)]++; 
				break;
			case 1:
				for (x = 0; x < _size; x++) 
					count[ZZ<_m>(Get(x ^ alpha)) -= Get(x)]++; 
				break;
			case 2:
				for (x = 0; x < _size; x++) 
					count[Get(ZZ<_n>(x) += alpha) ^= Get(x)]++; 
				break;
			case 3:
				for (x = 0; x < _size; x++) 
					count[ZZ<_m>(Get(ZZ<_n>(x) += alpha)) -= Get(x)]++; 
				break;
			default:
				assert(true);
			}
			if ((dc = count.Max()) > record)
				record = dc;
		}
		return record;
	}

	//!	Влияние единичных ошибок
	/*! Определяется максимальное значение характеристики PC1 для 
		невырожденных линейных комбинаций координатных функций. */
	size_t PC1() const
	{
		BFunc<_n> bf;
		size_t record = 0, pc1;
		Image wComb;
		while (wComb.Next())
		{
			GetCoordComb(wComb, bf);
			if ((pc1 = bf.PC1()) > record)
				record = pc1;
		}
		return record;
	}

// конструкторы
public:
	//! Конструктор по умолчанию
	/*! Создается функция с одинаковыми значениями-машинными словами 
		 valRight (нулевыми по умолчанию). */
	VFunc(word valRight = 0) : Func<_n, Image>(Word<_m>(valRight)) {}

	//! Конструктор по значению
	/*! Создается функция с одинаковыми значениями valRight. */
	VFunc(const Image& valRight) : Func<_n, Image>(valRight) {}

	//! Конструктор по значениям
	/*! Создается функция со значениями из массива valsRight. */
	VFunc(const Image (&valsRight)[_size]) : Func<_n, Image>(valsRight) {}

	//! Конструктор по значениям-машинным словам
	/*! Создается функция со значениями из массива машинных 
		слов valsRight. */
	VFunc(const word valsRight[_size])
	{	
		for (word x = 0; x < _size; x++)
			Set(x, valsRight[x]);
	}

	//! Конструктор копирования
	/*! Создается копия функции vfRight. */
	VFunc(const VFunc& vfRight) : Func<_n, Image>(vfRight) {}
};

/*******************************************************************************
Класс VSubst

Поддерживает манипуляции с биекциями на булевых n-ках.
*******************************************************************************/

template<size_t _n> class VSubst : public VFunc<_n, _n>
{
public:
	using typename VFunc<_n, _n>::Image;
protected:
	using VFunc<_n, _n>::_size;
protected:
	using VFunc<_n, _n>::Get;
	using VFunc<_n, _n>::Set;
// подстановка
public:
	//! Биективность?
	/*! Проверяется биективность преобразования. */
	bool IsBijection() const
	{	
		BFunc<_n> flags;
		for (word x = 0; x < _size; x++)
			flags[Get(x)] = true;
		return flags.Count(0) == 0;
	}

	//! Обращение
	/*! Подстановка заменяется обратной. */
	VSubst& Inverse()
	{	
		assert(IsBijection());
		VSubst<_n> save(*this);
		for (word x = 0; x < _size; x++)
			Set(save(x), Word<_n>(x));
		return *this;
	}

	//! Транспозиция
	/*! Выполняется перестановка образов от x и y. */
	VSubst& Transpose(word x, word y)
	{	
		if (x != y)
		{
			(*this)[x] ^= (*this)[y];
			(*this)[y] ^= (*this)[x];
			(*this)[x] ^= (*this)[y];
		}
		return *this;
	}

	//!	Тождественная подстановка
	/*!	Формируется тождественная подстановка. */
	void SetId()
	{	
		for (word x = 0; x < _size; x++)
			Set(x, Word<_n>(x));
	}

	//!	Тождественная подстановка?
	/*!	Проверяется тождественность подстановки. */
	bool IsId() const
	{	
		for (word x = 0; x < _size; x++)
			if (Get(x) != x)
				return false;
		return true;
	}

	//!	Полный цикл?
	/*!	Проверяется, что подстановка является полноцикловой. */
	bool IsFullCycle() const
	{	
		assert(IsBijection());
		word x = Get(0);
		size_t count = 1;
		while (x != 0)
			x = Get(x), ++count;
		return count == _size;
	}

	//!	Первая подстановка
	/*!	Формируется первая (в лексикографическом порядке) подстановка. */
	VSubst& First()
	{
		SetId();
		return *this;
	}

	//!	Следующая подстановка
	/*!	Формируется следующая (в лексикографическом порядке) подстановка. 
		Если подстановка является последней, то будет построена первая 
		подстановка.
		\return true, если построена следующая подстановка и false,
		если возвратились к первой. */
	bool Next()
	{
		// ищем с конца убывающую цепочку
		word i = _size - 2;
		while (i != WORD_MAX && Get(i) > Get(i + 1)) i--;
		// достигли начала, т.е. последняя подстановка?
		if (i == WORD_MAX)
		{
			First();
			return false;
		}
		// здесь s[i] < s[i + 1] > .... > s[_size - 1]
		word j = i + 1;
		while (j + 1 < _size && Get(j + 1) > Get(i)) j++;
		// теперь s[i] < s[j] и s[i] > s[j + 1]
		// выполняем транспозиции
		Transpose(i, j);
		j = _size;
		while (++i < --j)
			Transpose(i, j);
		// не последняя подстановка 
		return true;
	}

	//! Задать наудачу
	/*! Генерация случайной подстановки. */
	VSubst& Rand()
	{
		First();
		for (word x = _size; x != 0; x--)
			Transpose(x - 1, Env::Rand() % x);
		return *this;
	}

// операторы
public:
	//!	Присваивание
	/*!	Присваивание подстановке значения-набора образов valsRight. */
	VSubst& operator=(const Image (&valsRight)[_size])
	{
		VFunc<_n, _n>::operator=(valsRight);
		assert(IsBijection());
		return *this;
	}

	//!	Присваивание
	/*!	Присваивание подстановке значения-набора машинных слов 
		 valsRight. */
	VSubst& operator=(const word (&valsRight)[_size])
	{
		VFunc<_n, _n>::operator=(valsRight);
		for (word x = 0; x < _size; x++)
			Set(x, Word<_n>(valsRight[x]));
		assert(IsBijection());
		return *this;
	}

// конструкторы
public:
	//!	Конструктор по умолчанию
	/*!	Создается тождественная подстановка. */
	VSubst()
	{
		First();
	}

	//!	Конструктор по значениям
	/*!	Создается подстановка со значениями из массива valsRight. */
	VSubst(const Image (&valsRight)[_size]) : VFunc<_n, _n>(valsRight) 
	{
		assert(IsBijection());
	}

	//! Конструктор по значениям-машинным словам
	/*! Создается подстановка со значениями из массива машинных 
		слов valsRight. */
	VSubst(const word (&valsRight)[_size]) : VFunc<_n, _n>(valsRight)
	{	
		assert(IsBijection());
	}

	//! Конструктор копирования
	/*! Создается копия подстановки vsRight. */
	VSubst(const VSubst& vsRight) : VFunc<_n, _n>(vsRight) {}
};

} // namespace GF2

#endif // __GF2_FUNC
