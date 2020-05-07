/*
*******************************************************************************
\file mp.h
\brief Multivariate polynomials in GF(2)[x0,x1,...]
\project GF2 [algebra over GF(2)]
\created 2004.01.01
\version 2020.05.07
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file mp.h
\brief Многочлены от нескольких переменных

Модуль содержит описание и реализацию класса MP, поддерживающего манипуляции 
с многочленами от нескольких переменных над полем GF(2).
*******************************************************************************
*/

#ifndef __GF2_MP
#define __GF2_MP

#include "gf2/env.h"
#include "gf2/mm.h"
#include "gf2/mo.h"
#include <list>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>

namespace GF2 {

template<size_t _n, class _O = MOLex<_n>> class MP;
template<size_t _n, class _O = MOLex<_n>> class Ideal;

} // namespace GF2

#include "gf2/mi.h"

namespace GF2 {

/*!
*******************************************************************************
Класс MP

Поддерживает операции с многочленами от нескольких переменных. Многочлен 
является суммой мономов -- экземпляров класса MM. Многочлен параметризуется 
числом переменных _n и классом мономиального порядка _O, порожденным 
от MO<_n>. По умолчанию используется лексикографический порядок: 
_O = MOLex<_n>.

Класс порожден от списка std::list. Элементами списка являются различные 
мономы многочлена. При управлении списками предполагается, что мономы не 
повторяются и упорядочены по убыванию в порядке _O. Указанные ограничения 
на список поддерживает метод Normalize().

При переходе от списков к многочленам некоторые действия над списками 
получают определенный смысл. Например, слияние списков с удалением 
повторяющихся элементов соответствует сложению многочленов. 
Осмысленные операции над списками вынесены в отдельные методы класса MP 
либо в перегруженные операторы. Названия новых методов даются 
с заглавных букв. Не рекомендуется использовать generic методы 
списка std::list, которые начинаются со строчных букв.

Если все-таки требуется реализовать некоторые особые операции над мономами
многочленов, то следует прибегнуть к итераторам (подробнее см. STL):
\code
	MP<_n> poly;
	MP<_n>::iterator iter;
	word i;
	for (iter = poly.begin(), i = 1; iter != poly.end(); ++iter, ++i)
		(*iter) *= MM<_n>((5 * i) % _n, (7 * i) % _n);
\endcode
Обратим внимание, что в данном примере мы можем разрушить установленный
порядок мономов многочлена poly. Поэтому после цикла for следует вызвать
метод нормализации (поддержания порядка) Normalize().

При выполнении арифметических операций poly1 op poly2 возможны три ситуации:
-	порядки poly1 и poly2 совпадают по типу и параметрам
	(согласованные многочлены);
-	порядки poly1 и poly2 совпадают по типу, но не по параметрам
	(однотипные несогласованные многочлены);
-	порядки poly1 и poly2 различаются по типу
	(разнотипные несогласованные многочлены).
Данные ситуации по-возможности разбираются явно для повышения 
эффективности библиотеки (см., напр., реализацию operator+=()). 
Методы управления несогласованными списками снабжаются суффиксом NC 
(non-consistent). Возможно и неявное разрешение конфликтов порядков, 
которое поддерживается конструктором копирования и оператором 
присваивания.

Если poly1 и poly2 -- несогласованные многочлены, то при присваивании 
poly1 = poly2 в poly1 копируются только мономы poly2, а порядок не меняется. 
Если не ввести такое правило, то порядок poly1 будет разрушаться при 
присваивании однотипного несогласованного многочлена poly2, что оказывается 
не всегда удобным. С другой стороны, при создании poly1 разрушать нечего, 
поэтому	при конструировании poly1 по однотипному многочлену poly2 порядок 
poly2 копируется в poly1. Таким образом, следующие две строчки
\code
	MP<n, O> poly1(poly2);
	MP<n, O> poly1; poly1 = poly2;
\endcode
приведут, вообще говоря, к созданию разных poly1.

При сравнениях poly1 ><= poly2 сравниваются сначала старшие мономы многочленов,
затем вторые по старшинству и так далее, до нахождения первого различия или 
исчерпания мономов. Обязательно используется порядок poly1.

Определены операторы присваивания и конструкторы копирования, в которых 
в качестве параметров используются мономы и многочлены от другого числа 
переменных. Если в арифметических выражениях используются мономы и 
многочлены от разного числа переменных, то для разрешения неоднозначностей
может потребоваться явное приведение типов.

\todo Нужна ли перегрузка операторов с rvalue references?
*******************************************************************************
*/

template<size_t _n, class _O> class MP : public std::list<MM<_n>>
{
public:
	using typename std::list<MM<_n>>::iterator;
	using typename std::list<MM<_n>>::const_iterator;
	using std::list<MM<_n>>::assign;
	using std::list<MM<_n>>::begin;
	using std::list<MM<_n>>::clear;
	using std::list<MM<_n>>::end;
	using std::list<MM<_n>>::erase;
	using std::list<MM<_n>>::insert;
	using std::list<MM<_n>>::pop_front;
	using std::list<MM<_n>>::push_back;
	using std::list<MM<_n>>::size;
	using std::list<MM<_n>>::sort;
	using std::list<MM<_n>>::splice;
	using std::list<MM<_n>>::swap;
// число переменных
public:
	//! раскрытие числа переменных
	static constexpr size_t n = _n;

// мономиальный порядок
public:
	typedef _O Order;
protected:
	_O _order;
public:
	//! Получить мономиальный порядок
	/*! Возвращается установленный мономиальный порядок. */
	const _O& GetOrder() const
	{
		return _order;
	}

	//! Установить мономиальный порядок
	/*! Устанавливается мономиальный порядок oRight. 
		\remark имеет смысл устанавливать только параметризуемые 
		мономиальные порядки, поскольку все непараметризуемые порядки типа 
		_O обязательно совпадают. */
	void SetOrder(const _O& oRight)
	{
		if (!(_order == oRight))
			_order = oRight, Normalize();
	}

	//! Согласованность
	/*! Проверяется, что мономиальный порядок совпадает с порядком
		polyRight. */
	bool IsConsistent(const MP<_n, _O>& polyRight) const
	{
		return _order == polyRight.GetOrder();
	}

// управление списком мономов
public:
	//! Нормализация
	/*! Выполняется сортировка мономов по убыванию и удаление пар одинаковых
		мономов. */
	void Normalize()
	{	
		// сортировка
		sort(_order);
		// удаление повторов
		iterator iter = begin(), iterNext;
		while (iter != end() && ++(iterNext = iter) != end())
			if (*iter == *iterNext) iter = erase(erase(iter));
			else iter = iterNext; 
	}

	//! Нормализован?
	/*! Проверяется, что мономы не повторяются и отсортированы по убыванию.*/
	bool IsNormalized() const
	{	
		const_iterator iter = begin(), iterNext;
		while (iter != end() && ++(iterNext = iter) != end())
			if (_order.Compare(*iter, *iterNext) <= 0)
				return false;
			else iter = iterNext;
		return true;
	}

	//! Позиция монома
	/*! Определяется позиция, по которой моном mRight входит в многочлен
		(end(), если многочлен не содержит mRight). */
	const_iterator Find(const MM<_n>& mRight) const
	{	 
		// находим первую позицию в списке, вставка в которую mRight 
		// не нарушит порядок
		const_iterator iter = std::lower_bound(begin(), end(), mRight, _order);
		// нет совпадения?
		if (iter != end() && *iter != mRight)
			iter = end();
		return iter;
	}

	//! Позиция монома
	/*! Определяется позиция, по которой моном mRight входит в многочлен
		(end(), если многочлен не содержит mRight). */
	iterator Find(const MM<_n>& mRight)
	{	 
		// находим первую позицию в списке, вставка в которую mRight 
		// не нарушит порядок
		iterator iter = std::lower_bound(begin(), end(), mRight, _order);
		// нет совпадения?
		if (iter != end() && *iter != mRight)
			iter = end();
		return iter;
	}

	//! Обмен
	/*! Производится обмен списком мономов с согласованным многочленом 
		polyRight. */
	void Swap(MP& polyRight)
	{
		assert(IsConsistent(polyRight));
		swap(polyRight);
	}

	//! Добавление монома
	/*! Если моном mRight не входит в многочлен, то он добавляется к нему.*/
	void Union(const MM<_n>& mRight)
	{	
		// находим первую позицию в списке, вставка в которую mRight 
		// не нарушит порядок
		iterator iter = std::lower_bound(begin(), end(), mRight, _order);
		// нет mRight?
		if (iter == end() || *iter != mRight)
			insert(iter, mRight);
	}

	//! Добавление мономов
	/*! Добавляются мономы согласованного многочлена polyRight, 
		которые не входят в данный многочлен. */
	void Union(const MP& polyRight)
	{
		assert(IsConsistent(polyRight));
		// к самому себе?
		if (this == &polyRight)
			return;
		iterator iter = begin();
		const_iterator iterRight = polyRight.begin();
		while (iter != end() && iterRight != polyRight.end())
		{
			int cmp = _order.Compare(*iter, *iterRight);
			if (cmp > 0) ++iter;
			else
			{
				// вставить новый моном *iterRight перед *iter
				if (cmp < 0) iter = insert(iter, *iterRight);
				// перейти к следующим мономам
				++iter; ++iterRight;
			}
		}
		// добавить остаток polyRight
		if (iter == end()) 
			insert(end(), iterRight, polyRight.end());
	}

	//! Перенос мономов
	/*! Переносятся мономы согласованного многочлена polyRight, 
		которые не входят в данный многочлен. 
		\remark после выполнения метода polyRight содержит 
		только те мономы, которые входят в *this. */
	void UnionSplice(MP& polyRight)
	{
		assert(IsConsistent(polyRight));
		// к самому себе?
		if (this == &polyRight)
			return;
		iterator iter = begin(), iterRight = polyRight.begin(), iterNext;
		while (iter != end() && iterRight != polyRight.end())
		{
			int cmp = _order.Compare(*iter, *iterRight);
			if (cmp > 0) ++iter;
			else
			{
				// запомнить следующий элемент
				++(iterNext = iterRight);
				// *iter < *iterRight?
				if (cmp < 0)
				{
					// запомнить следующий моном polyRight
					++(iterNext = iterRight);
					// перенести *iterRight перед *iter	
					splice(iter, polyRight, iterRight);
					// перейти к следующему моному polyRight
					iterRight = iterNext;
				}
				// *iter == *iterRight?
				else ++iter, ++iterRight;
			}
		}
		// перенести остаток polyRight
		if (iter == end()) 
			splice(end(), polyRight, iterRight, polyRight.end());
	}

	//! Добавление мономов
	/*! Добавляются мономы несогласованного многочлена polyRight, 
		которые не входят в данный многочлен. */
	template<class _O1>
	void UnionNC(const MP<_n, _O1>& polyRight)
	{
		iterator iter = begin();
		typename MP<_n, _O1>::const_iterator iterRight = polyRight.begin();
		while (iterRight != polyRight.end())
		{
			// сравнить текущие элементы списков
			int cmp = (iter == end()) ? -1 : _order.Compare(*iter, *iterRight);
			if (cmp > 0) ++iter;
			else
			{
				// откат назад: плата за разноупорядочение списков
				// будем надеяться, что мономиальные порядки
				// близки между собой и откатов будет мало!
				while (cmp < 0 && iter != begin())
					cmp = _order.Compare(*--iter, *iterRight);
				// нашли *iterRight? 
				if (cmp == 0) 
					// дальше
					++iter;
				// *iterRight > всех мономов списка-назначения?
				else if (cmp < 0)
					// вставить *iterRight в начало списка
					iter = insert(iter, *iterRight);
				// *iter > *iterRight > *(++iter)?
				else if (cmp > 0)
					// вставить *iterRight после *iter
					iter = insert(++iter, *iterRight);
				// перейти к следующему моному
				++iterRight;
			}
		}
	}

	//! Исключение монома
	/*! Если моном mRight входит в многочлен, то он удаляется из него. */
	void Diff(const MM<_n>& mRight)
	{	
		// находим первую позицию в списке, вставка в которую mRight 
		// не нарушит порядок
		iterator iter = std::lower_bound(begin(), end(), mRight, _order);
		// нашли mRight?
		if (iter != end() || *iter == mRight)
			erase(iter);
	}

	//! Исключение мономов
	/*! Исключаются мономы согласованного многочлена polyRight, 
		которые входят в многочлен. */
	void Diff(const MP& polyRight)
	{	
		assert(IsConsistent(polyRight));
		// к самому себе?
		if (this == &polyRight)
		{
			SetEmpty();
			return;
		}
		iterator iter = begin();
		const_iterator iterRight = polyRight.begin();
		while (iter != end() && iterRight != polyRight.end())
		{
			int cmp = _order.Compare(*iter, *iterRight);
			if (cmp > 0) ++iter;
			else 
			{
				// удалить моном
				if (cmp == 0) iter = erase(iter);
				// к следующему
				++iterRight;
			}
		}
	}

	//! Исключение мономов
	/*! Исключаются мономы несогласованного многочлена polyRight, 
		которые входят в данный многочлен. */
	template<class _O1>
	void DiffNC(const MP<_n, _O1>& polyRight)
	{
		iterator iter = begin();
		typename MP<_n, _O1>::const_iterator iterRight = polyRight.begin();
		while (iterRight != polyRight.end())
		{
			// сравнить текущие элементы списков
			int cmp = (iter == end()) ? -1 : _order.Compare(*iter, *iterRight);
			if (cmp > 0) ++iter;
			else
			{
				// откат назад
				while (cmp < 0 && iter != begin())
					cmp = _order.Compare(*--iter, *iterRight);
				// нашли *iterRight?
				if (cmp == 0) 
					// удалить
					iter = erase(iter);
				// перейти к следующему моному
				++iterRight;
			}
		}
	}

	//! Исключающее добавление монома
	/*! Моном mRight исключается при вхождении в многочлен 
		и добавляется при отсутствии. */
	void SymDiff(const MM<_n>& mRight)
	{	
		// находим первую позицию в списке, вставка в которую mRight 
		// не нарушит порядок
		iterator iter = std::lower_bound(begin(), end(), mRight, _order);
		// нет mRight?
		if (iter == end() || *iter != mRight)
			insert(iter, mRight);
		else
			erase(iter);
	}

	//! Исключающее добавление мономов
	/*! Добавляются мономы согласованного многочлена polyRight, 
		которые не входят в многочлен и исключаются мономы, которые 
		входят в многочлен. */
	void SymDiff(const MP& polyRight)
	{	
		assert(IsConsistent(polyRight));
		// к самому себе?
		if (this == &polyRight)
		{
			SetEmpty();
			return;
		}
		iterator iter = begin();
		const_iterator iterRight = polyRight.begin();
		while (iter != end() && iterRight != polyRight.end())
		{
			int cmp = _order.Compare(*iter, *iterRight);
			if (cmp > 0) ++iter;
			else 
			{
				// вставить новый моном
				if (cmp < 0) iter = insert(iter, *iterRight);
				// исключить повтор
				else iter = erase(iter);
				// к следующему 
				++iterRight;
			}
		}
		// добавить остаток polyRight
		if (iter == end())
			insert(end(), iterRight, polyRight.end());
	}

	//! Исключающий перенос мономов
	/*! Переносятся мономы согласованного многочлена polyRight, 
		которые не входят в многочлен и исключаются мономы, которые 
		входят в многочлен. 
		\remark после переноса polyRight содержит только те мономы,
		которые входят в *this. */
	void SymDiffSplice(MP& polyRight)
	{	
		assert(IsConsistent(polyRight));
		// к самому себе?
		if (this == &polyRight)
		{
			SetEmpty();
			return;
		}
		iterator iter = begin(), iterRight = polyRight.begin(), iterNext;
		while (iter != end() && iterRight != polyRight.end())
		{
			int cmp = _order.Compare(*iter, *iterRight);
			if (cmp > 0) ++iter;
			else 
			{
				// *iter < *iterRight?
				if (cmp < 0)
				{
					// запомнить следующий моном polyRight
					++(iterNext = iterRight);
					// перенести *iterRight перед *iter	
					splice(iter, polyRight, iterRight);
					// перейти к следующему моному polyRight
					iterRight = iterNext;
				}
				// *iter == *iterRight?
				else iter = erase(iter), ++iterRight;
			}
		}
		// добавить остаток polyRight
		if (iter == end())
			splice(end(), polyRight, iterRight, polyRight.end());
	}

	//! Исключающее добавление мономов
	/*! Добавляются мономы несогласованного многочлена polyRight, 
		которые не входят в данный многочлен, 
		и исключаются мономы, которые входят в данный многочлен. */
	template<class _O1>
	void SymDiffNC(const MP<_n, _O1>& polyRight)
	{
		iterator iter = begin();
		typename MP<_n, _O1>::const_iterator iterRight = polyRight.begin();
		while (iterRight != polyRight.end())
		{
			// сравнить текущие элементы списков
			int cmp = (iter == end()) ? -1 : _order.Compare(*iter, *iterRight);
			if (cmp > 0) ++iter;
			else
			{
				// откат назад
				while (cmp < 0 && iter != begin())
					cmp = _order.Compare(*--iter, *iterRight);
				// нашли *iterRight?
				if (cmp == 0) 
					// удалить
					iter = erase(iter);
				// *iterRight > всех мономов списка-назначения?
				else if (cmp < 0)
					// вставить *iterRight в начало списка
					iter = insert(iter, *iterRight);
				// *iter > *iterRight > *(++iter)?
				else if (cmp > 0)
					// вставить *iterRight после *iter
					iter = insert(++iter, *iterRight);
				// перейти к следующему моному
				++iterRight;
			}
		}
	}

	//! Сравнение с многочленом
	/*! Выполняется сравнение с согласованным многочленом polyRight. 
		\return -1 (<), 0 (=), 1(>). */
	int Compare(const MP& polyRight) const
	{
		assert(IsConsistent(polyRight));
		// цикл по мономам
		const_iterator iter = begin(), iterRight = polyRight.begin();
		for (; iter != end() && iterRight != polyRight.end(); ++iter, ++iterRight)
			if (int cmp = _order.Compare(*iter, *iterRight))
				return cmp;
		// проверка хвостов
		if (iter == end()) 
			return iterRight == polyRight.end() ? 0 : -1;
        return 1;
	}

	//! Сравнение с многочленом
	/*! Выполняется сравнение с несогласованным многочленом polyRight. 
		\return -1 (<), 0 (=), 1(>). */
	template<class _O1>
	int CompareNC(const MP<_n, _O1>& polyRight) const
	{
		// меняем порядок
		MP poly(polyRight);
		poly.SetOrder(_order);
		// сравниваем
		return Compare(poly);
	}

	//! Упаковка
	/*! Переменные, которые входят в mMask, заменяются на переменные x0, \с x1,... 
		Остальные переменные исключаются. */
	void Pack(const MM<_n>& mMask)
	{
		for (iterator iter = begin(); iter != end(); ++iter)
			iter->Pack(mMask);
		Normalize();
	}

	//! Распаковка
	/*! Переменные \с x0, x1,... заменяются на переменные, которые входят в mMask. */
	void Unpack(const MM<_n>& mMask)
	{
		for (iterator iter = begin(); iter != end(); ++iter)
			iter->Unpack(mMask);
		Normalize();
	}

// базовые операции
public:
	//! Число мономов
	/*! Возвращается число мономов в многочлене. */
	size_t Size() const
	{	
		return (size_t)size();
	}

	//! Обнуление
	//! Удаляются все мономы. */
	void SetEmpty()
	{	
		clear();
	}

	//! Нулевой моногочлен?
	//! Возвращается признак отстутствия в многочлене мономов. */
	bool IsEmpty() const
	{	
		return Size() == 0;
	}

	//! Содержит моном?
	/*! Выполняется признак вхождения в многочлен монома mRight. */
	bool IsContain(const MM<_n>& mRight) const
	{	
		return Find(mRight) != end();
	}

	//! Степень
	/*! Определяется степень многочлена. */
	int Deg() const
	{
		// нулевой многочлен?
		if (IsEmpty()) return -1;
		// максимум степеней мономов
		int ret = 0, deg;
		for (const_iterator iter = begin(); iter != end(); ++iter)
			if ((deg = iter->Deg()) > ret) ret = deg;
		return ret;
	}

	//! Старший моном
	/*! Определяется старший (в установленном порядке) моном многочлена. 
		\pre !IsEmpty(). */
	const MM<_n>& LM() const
	{	
		assert(!IsEmpty());
		return *begin();
	}

	//! Без старшего монома
	/*! Удаляется старший (в установленном порядке) моном многочлена. 
		\pre !IsEmpty(). */
	void PopLM()
	{	
		assert(!IsEmpty());
		pop_front();
	}

	//! Значение
	/*! Определяется значение многочлена при подстановке на места переменных
		символов слова val. */
	bool Calc(const WW<_n>& val) const
	{
		bool ret = 0;
		for (const_iterator iter = begin(); iter != end(); ++iter)
			ret ^= iter->Calc(val);
		return ret;
	}

	//! Несущественные переменные?
	/*! Проверяется, что переменные, заданые ненулевыми символами vars,
		являются несущественными. */
	bool IsInsufficient(const WW<_n>& vars) const
	{
		WW<_n> mask;
		for (const_iterator iter = begin(); iter != end(); ++iter)
			if (!((mask = vars) &= *iter).IsAllZero())
				return false;
		return true;
	}

	//! Задать наудачу
	/*! Генерация случайного многочлена. */
	MP& Rand()
	{
		SetEmpty();
		MM<_n> m;
		do if (Env::Rand() & 1)
			insert(begin(), m);
		while (_order.Next(m));
		return *this;
	}

// операции
public:
	//! Присваивание
	/*! Присваивание многочлену значения-монома mRight. */
	MP& operator=(const MM<_n>& mRight)
	{
		clear();
		push_back(mRight);
		return *this;
	}

	//! Присваивание
	/*! Присваивание многочлену значения-монома mRight с другим число переменных. */
	template<size_t _m>
	MP& operator=(const MM<_m>& mRight)
	{
		clear();
		push_back(MM<_n>(mRight));
		return *this;
	}

	//! Присваивание
	/*! Присваивание многочлену значения-константы cRight. */
	MP& operator=(bool cRight)
	{	
		clear();
		if (cRight) push_back(MM<_n>());
		return *this;
	}

	//! Присваивание
	/*! Присваивание многочлену однотипного значения-многочлена polyRight. 
		\remark присваиваются только мономы многочлена polyRight, но не
		(возможно отличающиеся) параметры порядка polyRight. */
	MP& operator=(const MP& polyRight)
	{	
		if (&polyRight != this)
		{
			std::list<MM<_n>>::operator=(polyRight);
			// нормализация без присваивания порядка!
			if (!IsConsistent(polyRight))
				Normalize();
		}
		return *this;
	}

	//! Захват
	/*! Захватывается временный многочлен polyRight.
		\remark присваиваются только мономы многочлена polyRight, но не
		(возможно отличающиеся) параметры порядка polyRight. */
	MP& operator=(MP&& polyRight)
	{
		if (&polyRight != this)
		{
			std::list<MM<_n>>::operator=(std::move(polyRight)); 
			// нормализация без присваивания порядка!
			if (!IsConsistent(polyRight))
				Normalize();
		}
		return *this;
	}

	//! Присваивание
	/*! Присваивание многочлену значения-многочлена polyRight с произвольным
		мономиальным порядком. */
	template<class _O1>
	MP& operator=(const MP<_n, _O1>& polyRight)
	{	
		assign(polyRight.begin(), polyRight.end());
		Normalize();
		return *this;
	}

	//! Присваивание
	/*! Присваивание многочлену значения-многочлена polyRight с другим
		числом переменных. */
	template<size_t _m, class _O1>
	MP& operator=(const MP<_m, _O1>& polyRight)
	{	
		typename MP<_m, _O1>::const_iterator iter;
		for (iter = polyRight.begin(); iter != polyRight.end(); ++iter)
			SymDiff(MM<_n>(*iter));
		return *this;
	}

	//! Значение
	/*! Определяется значение многочлена при подстановке на места переменных
		символов слова val. */
	bool operator()(const WW<_n>& val) const
	{
		return Calc(val);
	}

// аддитивные операции
public:
	//! Плюс
	/*! Унарный плюс (пустой оператор). */
	MP& operator+()
	{	
		return *this;
	}

	//! Прибавление монома
	/*! К многочлену прибавляется моном mRight. */
	MP& operator+=(const MM<_n>& mRight)
	{	
		SymDiff(mRight);
		return *this;
	}

	//! Прибавление константы
	/*! К многочлену прибавляется константа cRight. */
	MP& operator+=(bool cRight)
	{	
		if (cRight) operator+=(MM<_n>());
		return *this;
	}

	//! Прибавление однотипного многочлена
	/*! К многочлену прибавляется многочлен polyRight с тем же типом
		мономиального порядка (но, возможно, с другими параметрами). */
	MP& operator+=(const MP& polyRight)
	{	
		// многгочлены согласованы?
		if (IsConsistent(polyRight))
			// да, эффективное слияние списков
			SymDiff(polyRight);
		else
			// нет, неэффективное слияние списков
			SymDiffNC(polyRight);
		return *this;
	}

	//! Прибавление многочлена
	/*! К многочлену прибавляется многочлен polyRight с другим типом 
		мономиального порядка. */
	template<class _O1>
	MP& operator+=(const MP<_n, _O1>& polyRight)
	{	
		SymDiffNC(polyRight);
		return *this;
	}

// класс Geobucket
public:
	/*!
	***************************************************************************
	Класс Geobucket

	-#	Класс Geobucket<_d> поддерживает специальную структуру данных:
		"последовательность корзин с геометрически растущими размерами" 
		(\b geobucket) [Yan T. The Geobucket Data Structure for Polynomials, 
		J. of Symb. Comp., v.25 (1998)]. Корзинами являются многочлены как 
		упорядоченные списки мономов.
	-#	Использование geobucket позволяет повысить скорость следующего 
		алгоритмического примитива:
		\code
		for(...) 
			выделить старший моном многочлена p,
			p += некоторый многочлен;
		\endcode
	-#	Скорость роста размеров корзин определяет параметр шаблона _d.
		Асимптотически время сложения линейно зависит от (d+1)/\ln d,
		a суммарное время сложения и выделения старшего монома 
		линейно зависит от (d+2)/\ln d. 
		Оптимальным для сложения является выбор _d = 3.59, 
		а для последовательного сложения и выделения старшего монома -- 
		выбор d = 4.31. В реализации поддержаны только целые _d.
	-#	Мономиальный порядок передается в конструкторе через ссылку на объект
		поддержки порядка либо через ссылку на многочлен с определенным 
		порядком. Все добавляемые в \b geobucket многочлены должны быть 
		согласованы с первоначальным порядком конструктора.
	-#	Для повышения эффективности мономы добавляемого многочлена poly
		не копируются, а перемещаются в корзины. Многочлен poly "отдает"
		часть своих мономов и, таким образом, меняется. 
		Об этом не следует забывать при программировании.
	***************************************************************************
	*/
	template<word _d> class Geobucket
	{
	// корзины
	protected:
		std::vector<MP> _buckets; // корзины
		std::vector<size_t> _maxsizes; // максимальные размеры корзин

		//! Добавление корзины
		/*! Добавляется новая пустая корзина. */
		void NewBucket()
		{
			size_t size = (size_t)_buckets.size();
			_buckets.resize(size + 1);
			_buckets[size].SetOrder(_buckets[size - 1].GetOrder());
			_maxsizes.push_back(_d * _maxsizes[size - 1]);
		}
	
	// конструктор
	public:
		//! Конструктор
		/*! Конструктор по мономиальному порядку orderRight. */
		Geobucket(const _O& orderRight)
		{
			_buckets.resize(1);
			_buckets[0].SetOrder(orderRight);
			_maxsizes.push_back(_d);
		}

		//! Конструктор
		/*! Конструктор по многочлену polyRight.
			\remark \с polyRight меняется! */
		Geobucket(MP& polyRight)
		{
			_buckets.resize(1);
			_buckets[0].SetOrder(polyRight.GetOrder());
			_maxsizes.push_back(_d);
			SymDiffSplice(polyRight);
		}

	// операции
	public:
		//! Исключающее добавление монома
		/*! Добавляется моном mRight, если он остутствует в \b geobucket,
			либо добавляется, если присутствует. */
		void SymDiff(const MM<_n>& mRight)
		{
			size_t i = 0;
			// добавляем мономы в первую корзину
			_buckets[0].SymDiff(mRight);
			while (_buckets[0].Size() > _maxsizes[i])
			{
				// корзины закончились?
				if (i + 1 == _maxsizes.size()) NewBucket();
				// перераспределяем мономы по корзинам 
				_buckets[0].SymDiffSplice(_buckets[++i]);
				_buckets[i].SetEmpty();
			}
			// меняем содержимое корзин
			_buckets[i].Swap(_buckets[0]);
		}

		//! Исключающее добавление многочлена
		/*! Добавляются мономы согласованного многочлена polyRight, 
			которые отсутствуют в \b geobucket, и исключаются присутствующие 
			мономы. */
		void SymDiffSplice(MP& polyRight)
		{
			// поиск подходящей корзины 
			size_t i = 0, j;
			for (; polyRight.Size() > _maxsizes[i];)
				// корзины закончились?
				if (++i == _maxsizes.size()) NewBucket();
			// добавляем мономы в найденную корзину
			_buckets[j = i].SymDiffSplice(polyRight);
			while (_buckets[j].Size() > _maxsizes[i])
			{
				// корзины закончились?
				if (i + 1 == _maxsizes.size()) NewBucket();
				// перераспределяем мономы по корзинам 
				_buckets[j].SymDiffSplice(_buckets[++i]);
				_buckets[i].SetEmpty();
			}
			// меняем содержимое i-й и j-й корзин
			_buckets[i].Swap(_buckets[j]);
		}

		//! Старший моном
		/*! Определяется и удаляется старший моном многочлена, 
			размещенного в корзинах.
			\return \b false, если многочлен нулевой, и \b true, 
			если многочлен ненулевой и старший моном занесен в lm. */
		bool PopLM(MM<_n>& lm)
		{
			int cmp;
			size_t i = SIZE_MAX, j = (size_t)_buckets.size();
			const _O& o = _buckets[0].GetOrder();
			do
			{
				// пропускаем пустые корзины
				if (_buckets[--j].IsEmpty()) continue;
				// новый старший моном?
				if (i == SIZE_MAX || (cmp = o.Compare(_buckets[j].LM(), lm)) > 0)
					lm = _buckets[i = j].LM();
				// одинаковые мономы?
				else if (cmp == 0) 
				{
					// удаляем мономы из корзин и начинаем все сначала
					_buckets[i].PopLM(), _buckets[j].PopLM();
					i = SIZE_MAX, j = (size_t)_buckets.size();
				}
			}
			while (j != 0);
			// моногочлен ненулевой?
			if (i != SIZE_MAX) _buckets[i].PopLM();
			// признак успеха
			return i != SIZE_MAX;
		}

		//! Сборка многочлена
		/*! Все мономы корзин суммируются в согласованном многочлене 
			polyRight. */
		void Mount(MP& polyRight)
		{
			polyRight.SetEmpty();
			for (size_t i = 0; i < _buckets.size();)
				polyRight.SymDiffSplice(_buckets[i++]);
		}
	};

// мультипликативные примитивы
public:
	//! Умножение на многочлен
	/*! Выполняется стандартное умножение на многочлен polyRight. */
	template<class _O1>
	void MultClassic(const MP<_n, _O1>& polyRight)
	{
		MP polySave(*this), poly(_order);
		typename MP<_n, _O1>::const_iterator iter;
		for (clear(), iter = polyRight.end(); iter != polyRight.begin();)
			SymDiffSplice((poly = polySave) *= *--iter);
	}

	//! Умножение на многочлен
	/*! Выполняется умножение на многочлен polyRight с использованием
		\b geobucket. */
	template<class _O1>
	void Mult(const MP<_n, _O1>& polyRight)
	{
		// поскольку операция выбора старшего монома не задействована,
		// коэффициент роста geobucket выберем равным 3
		Geobucket<3> gb(_order);
		MP poly;
		// цикл по мономам polyRight
		typename MP<_n, _O1>::const_iterator iterRight;
		for (iterRight = polyRight.end(); iterRight != polyRight.begin();)
			gb.SymDiffSplice((poly = *this) *= *--iterRight);
		// сборка
		gb.Mount(*this);
	}

	//! Остаток от деления на многочлен
	/*! Определяется остаток от деления на ненулевой многочлен polyRight.
		\return \b true, если остаток отличается от делимого. */
	template<class _O1>
	bool ModClassic(const MP<_n, _O1>& polyRight)
	{	
		bool changed = false;
		MP poly(_order);
		iterator iter = begin();
		do
			if (iter->IsDivisibleBy(polyRight.LM()))
			{
				SymDiffSplice((poly = polyRight) *= *iter / polyRight.LM());
				iter = begin();
				changed = true;
			}
			else ++iter;
		while (iter != end());
		return changed;
	}

	//! Остаток от деления на многочлен
	/*! Определяется остаток от деления на ненулевой многочлен polyRight. 
		Используется структура geobucket. 
		\return \b true, если остаток отличается от делимого. */
	template<class _O1>
	bool Mod(const MP<_n, _O1>& polyRight)
	{	
		// поскольку операция выбора старшего монома задействована,
		// коэффициент роста geobucket выберем равным 4
		Geobucket<4> gb(*this);
		// будем сохранять в this остаток
		SetEmpty();
		// цикл деления
		bool changed = false;
		MM<_n> lm;
		MP poly(_order);
		while (gb.PopLM(lm))
			// делимость старших мономов?
			if (lm.IsDivisibleBy(polyRight.LM()))
			{
				changed = true;
				// готовим множитель
				(poly = polyRight).PopLM();
				// перемножаем и добавляем в geobucket
                gb.SymDiffSplice(poly *= (lm /= polyRight.LM()));
			}
			else
				// заносим lm в остаток
				push_back(lm);
		return changed;
	}

	//! Частное от деления на многочлен
	/*! Определяются частное от деления на ненулевой многочлен polyRight.*/
	template<class _O1>
	void DivClassic(const MP<_n, _O1>& polyRight)
	{	
		// готовим временный многочлен и многочлен для остатка
		MP poly(_order), polyMod(_order);
		polyMod.Swap(*this);
		// цикл деления
		iterator iter = polyMod.begin();
		MM<_n> m;
		do
			if (iter->IsDivisibleBy(polyRight.LM()))
			{
				(m = *iter) /= polyRight.LM();
				// меняем остаток
				polyMod.SymDiffSplice((poly = polyRight) *= m);
				// заносим в хвост новый моном частного
				push_back(m);
				// все снова
				iter = polyMod.begin();
			}
			else ++iter;
		while (iter != polyMod.end());
	}

	//! Частное от деления на многочлен
	/*! Находится частное от деления на ненулевой многочлен polyRight.
		Используется структура \b geobucket */
	template<class _O1>
	void Div(const MP<_n, _O1>& polyRight)
	{	
		// поскольку операция выбора старшего монома задействована,
		// коэффициент роста geobucket выберем равным 4
		Geobucket<4> gb(*this);
		// будем сохранять в this частное
		SetEmpty();
		// цикл деления
		MM<_n> lm;
		MP poly(_order);
		while (gb.PopLM(lm))
			// делимость старших мономов?
			if (lm.IsDivisibleBy(polyRight.LM()))
			{
				// готовим множитель
				(poly = polyRight).PopLM();
				// перемножаем и добавляем в geobucket
                gb.SymDiffSplice(poly *= (lm /= polyRight.LM()));
				// заносим lm в частное
				push_back(lm);
			}
	}

	//! Признак делимости
	/*! Проверяется, что все мономы многочлена делятся на mRight. */
	bool IsDivisibleBy(const MM<_n>& mRight) const
	{
		for (const_iterator iter = begin(); iter != end(); ++iter)
			if (!iter->IsDivisibleBy(mRight))
				return false;
		return true;
	}

	//! Замена переменной
	/*! Выполняется замена вхождений переменной с номером pos 
		на многочлен polyReplace. */
	template<class _O1>
	void ReplaceClassic(size_t pos, const MP<_n, _O1>& polyReplace)
	{	
		MM<_n> m;
		MP polyResult(_order), poly(_order);
		for (iterator iter = begin(); iter != end(); ++iter)
			// переменная pos входит в моном *iter?
			if (iter->Test(pos))
			{	
				(m = *iter).Flip(pos);
				polyResult.SymDiffSplice((poly = polyReplace) *= m);
			}
			else polyResult.SymDiff(*iter);
		Swap(polyResult);
	}

	//! Замена переменной
	/*! Выполняется замена вхождений переменной с номером pos 
		на многочлен polyReplace. Используется структура \b geobucket. */
	template<class _O1>
	void Replace(size_t pos, const MP<_n, _O1>& polyReplace)
	{	
		MM<_n> m;
		MP poly(_order);
		Geobucket<3> gb(_order);
		for (iterator iter = begin(); iter != end(); ++iter)
			// переменная pos входит в моном *iter?
			if (iter->Test(pos))
			{	
				(m = *iter).Flip(pos);
				gb.SymDiffSplice((poly = polyReplace) *= m);
			}
			else gb.SymDiff(*iter);
		gb.Mount(*this);
	}

	//! Замена переменной
	/*! Выполняется замена вхождений переменной с номером pos 
		на переменную с номером posNew. */
	void Replace(size_t pos, size_t posNew)
	{	
		if (pos == posNew) return;
		for (iterator iter = begin(); iter != end(); ++iter)
			// переменная pos входит в моном *iter?
			if (iter->Test(pos))
				iter->Flip(pos), iter->Set(posNew, 1);
		Normalize();
	}

	//! Перестановка переменных
	/*! Выполняется перестановка переменных в соответствии с таблицей pi. */
	void Permute(const size_t pi[_n])
	{	
		for (iterator iter = begin(); iter != end(); ++iter)
			iter->Permute(pi);
		Normalize();
	}

	//! Установка значения переменной
	/*! Выполняется замена вхождений переменной с номером pos 
		на константу val. */
	void Set(size_t pos, bool val)
	{	
		if (val == 0)
		{
			for (iterator iter = begin(); iter != end();)
				if (iter->Test(pos)) iter = erase(iter);
				else ++iter;
		}
		else
		{
			for (iterator iter = begin(); iter != end(); ++iter)
				iter->Set(pos, 0);
			Normalize();
		}
	}

	//! S-многочлен
	/*! Определяется S-многочлен согласованной пары (poly1, poly2)
		ненулевых многочленов. 
		\pre многочлены пары должны отличаться от *this. */
	MP& SPoly(const MP& poly1, const MP& poly2)
	{
		// предусловия
		assert(IsConsistent(poly1) && IsConsistent(poly2));
		assert(this != &poly1 && this != &poly2);
		// сбор мономов
		MM<_n> lm, m;
		const_iterator iter;
		SetEmpty();
		lm.LCM(poly1.LM(), poly2.LM()) /= poly1.LM();
		for (iter = poly1.begin(); ++iter != poly1.end();)
			push_back((m = *iter) *= lm);
		lm.LCM(poly1.LM(), poly2.LM()) /= poly2.LM();
		for (iter = poly2.begin(); ++iter != poly2.end();)
			push_back((m = *iter) *= lm);
		// нормализация
		Normalize();
		return *this;
	}

	//! S-многочлен
	/*! Определяется S-многочлен пары (x_i^2-x_i, poly),
		где poly -- согласованный ненулевой многочлен, 
		отличный от *this. */
	MP& SPoly(size_t i, const MP& poly)
	{
		// предусловия
		assert(this != &poly && IsConsistent(poly));
		// сбор мономов
		MM<_n> m;
		SetEmpty();
		for (const_iterator iter = poly.begin(); iter != poly.end(); ++iter)
			(m = *iter).Set(i, 1), push_back(m);
		// нормализация
		Normalize();
		return *this;
	}

	//! S-многочлен
	/*! Определяется S-многочлен согласованной пары (*this, poly)
		различных ненулевых многочленов. */
	MP& SPoly(const MP& poly)
	{
		// предусловия
		assert(IsConsistent(poly) && this != &poly);
		// сбор мономов
		MM<_n> lm, m;
		(m = lm.LCM(LM(), poly.LM())) /= LM();
		PopLM();
		for (iterator iter = begin(); iter != end(); ++iter)
			*iter *= m;
		lm /= poly.LM();
		for (const_iterator iter = poly.begin(); ++iter != poly.end();)
			push_back((m = *iter) *= lm);
		// нормализация
		Normalize();
		return *this;
	}

	//! S-многочлен
	/*! Определяется S-многочлен пары (x_i^2-x_i, *this). */
	MP& SPoly(size_t i)
	{
		for (iterator iter = begin(); iter != end(); ++iter)
			iter->Set(i, 1);
		Normalize();
		return *this;
	}

// мультипликативные операции
public:
	//! Умножение на моном
	/*! Выполняется умножение на моном mRight. */
	MP& operator*=(const MM<_n>& mRight)
	{	
		for (iterator iter = begin(); iter != end(); ++iter)
			(*iter) *= mRight;
		// при умножении могут появиться повторяющиеся мономы 
		// и может нарушиться порядок
		Normalize();
		return *this;
	}

	//! Умножение на многочлен
	/*! Выполняется умножение на многочлен polyRight. */
	template<class _O1>
	MP& operator*=(const MP<_n, _O1>& polyRight)
	{
		Mult(polyRight);
		return *this;
	}

	//! Остаток от деления на моном
	/*! Определяется остаток от деления на моном mRight, т.е. 
		исключаются мономы, которые делятся на mRight. */
	MP& operator%=(const MM<_n>& mRight)
	{	
		for (iterator iter = begin(); iter != end();)
			if (iter->IsDivisibleBy(mRight)) 
				iter = erase(iter);
			else ++iter;
		return *this;
	}

	//! Остаток от деления на многочлен
	/*! Определяется остаток от деления на ненулевой многочлен polyRight.*/
	template<class _O1>
	MP& operator%=(const MP<_n, _O1>& polyRight)
	{	
		Mod(polyRight);
		return *this;
	}

	//! Частное от деления на моном
	/*! Определяется частное от деления на моном mRight. */
	MP& operator/=(const MM<_n>& mRight)
	{	
		for (iterator iter = begin(); iter != end();)
			if (iter->IsDivisibleBy(mRight)) 
				*iter++ /= mRight;
			else iter = erase(iter);
		return *this;
	}

	//! Частное от деления на многочлен
	/*! Определяется частное от деления на ненулевой многочлен polyRight.*/
	template<class _O1>
	MP& operator/=(const MP<_n, _O1>& polyRight)
	{	
		Div(polyRight);
		return *this;
	}

// сравнения
public:
	//! Равенство моному
	/*! Проверяется равенство моному mRight. */
	bool operator==(const MM<_n>& mRight) const
	{	
		return Size() == 1 && *begin() == mRight;
	}

	//! Равенство константе
	/*! Проверяется равенство константе cRight. */
	bool operator==(bool cRight) const
	{	
		return cRight ? Size() == 1 && begin()->IsAllZero() : IsEmpty();
	}

	//! Равенство однотипному многочлену
	/*! Проверяется равенство многочлену polyRight с тем же типом
		мономиального порядка (но, возможно, с другими параметрами). */
	bool operator==(const MP<_n, _O>& polyRight) const
	{	
		// немного упростим сравнение
		if (Size() != polyRight.Size()) return false;
		// порядки совпадают по параметрам?
		if (_order == polyRight.GetOrder())
			// да, эффективное сравнение
			return Compare(polyRight) == 0;
		else
			// нет, неэффективное сравнение
			return CompareNC(polyRight) == 0;
	}

	//! Равенство однотипному многочлену
	/*! Проверяется равенство многочлену polyRight с другим типом
		мономиального порядка. */
	template<class _O1>
	bool operator==(const MP<_n, _O1>& polyRight) const
	{	
		if (Size() != polyRight.Size()) return false;
		return CompareNC(polyRight) == 0;
	}

	//! Неравенство моному
	/*! Проверяется неравенство моному mRight. */
	bool operator!=(const MM<_n>& mRight) const
	{	
		return Size() != 1 || *begin() != mRight;
	}

	//! Неравенство константе
	/*! Проверяется неравенство константе cRight. */
	bool operator!=(bool cRight) const
	{	
		return cRight ? Size() != 1 || !begin()->IsAllZero() : Size() != 0;
	}

	//! Неравенство многочлену
	/*! Проверяется неравенство многочлену polyRight с тем же типом
		мономиального порядка. */
	bool operator!=(const MP& polyRight) const
	{	
		// немного упростим сравнение
		if (Size() != polyRight.Size()) return true;
		// порядки совпадают по параметрам?
		if (_order == polyRight.GetOrder())
			// да, эффективное сравнение
			return Compare(polyRight) != 0;
		else
			// нет, неэффективное сравнение
			return CompareNC(polyRight) != 0;
	}

	//! Неравенство многочлену
	/*! Проверяется неравенство многочлену polyRight с другим типом
		мономиального порядка. */
	template<class _O1>
	bool operator!=(const MP<_n, _O1>& polyRight) const
	{	
		if (Size() != polyRight.Size()) return true;
		return CompareNC(polyRight) != 0;
	}

	//! Меньше монома?
	/*! Проверяется, что данный многочлен меньше многочлена-монома mRight.*/
	bool operator<(const MM<_n>& mRight) const
	{	
		return IsEmpty() || _order.Compare(*begin(), mRight) < 0;
	}

	//! Меньше константы?
	/*! Проверяется, что данный многочлен меньше многочлена-константы cRight.*/
	bool operator<(bool cRight) const
	{	
		return cRight ? Size() == 0 : false;
	}

	//! Меньше многочлена?
	/*! Проверяется, что данный многочлен меньше многочлена polyRight 
		с тем же типом мономиального порядка. */
	bool operator<(const MP& polyRight) const
	{	
		// порядки совпадают по параметрам?
		if (_order == polyRight.GetOrder())
			// да, эффективное сравнение
			return Compare(polyRight) < 0;
		else
			// нет, неэффективное сравнение
			return CompareNC(polyRight) < 0;
	}

	//! Меньше многочлена?
	/*! Проверяется, что данный многочлен меньше многочлена polyRight 
		с другим типом мономиального порядка. */
	template<class _O1>
	bool operator<(const MP<_n, _O1>& polyRight) const
	{	
		return CompareNC(polyRight) < 0;
	}

	//! Не больше монома?
	/*! Проверяется, что данный многочлен не больше многочлена-монома mRight.*/
	bool operator<=(const MM<_n>& mRight) const
	{	
		return IsEmpty() || begin()->IsAllZero();
	}

	//! Не больше константы?
	/*! Проверяется, что данный многочлен не больше 
		многочлена-константы cRight.*/
	bool operator<=(bool cRight) const
	{	
		return cRight ? IsEmpty() || begin()->IsAllZero() : IsEmpty();
	}

	//! Не больше многочлена?
	/*! Проверяется, что данный многочлен не больше многочлена polyRight 
		с тем же типом мономиального порядка. */
	bool operator<=(const MP& polyRight) const
	{	
		// порядки совпадают по параметрам?
		if (_order == polyRight.GetOrder())
			// да, эффективное сравнение
			return Compare(polyRight) <= 0;
		else
			// нет, неэффективное сравнение
			return CompareNC(polyRight) <= 0;
	}

	//! Не больше многочлена?
	/*! Проверяется, что данный многочлен не больше многочлена polyRight 
		с другим типом мономиального порядка. */
	template<class _O1>
	bool operator<=(const MP<_n, _O1>& polyRight) const
	{	
		return Compare(polyRight) <= 0;
	}

	//! Больше монома?
	/*! Проверяется, что данный многочлен больше многочлена-монома mRight.*/
	bool operator>(const MM<_n>& mRight) const
	{	
		return !IsEmpty() && _order.Compare(*begin(), mRight) > 0;
	}

	//! Больше константы?
	/*! Проверяется, что данный многочлен больше 
		многочлена-константы cRight.*/
	bool operator>(bool cRight) const
	{	
		return cRight ? !IsEmpty() && !begin()->IsAllZero() : false;
	}

	//! Больше многочлена?
	/*! Проверяется, что данный многочлен больше многочлена polyRight 
		с тем же типом мономиального порядка. */
	bool operator>(const MP& polyRight) const
	{	
		// порядки совпадают по параметрам?
		if (_order == polyRight.GetOrder())
			// да, эффективное сравнение
			return Compare(polyRight) > 0;
		else
			// нет, неэффективное сравнение
			return CompareNC(polyRight) > 0;
	}

	//! Больше многочлена?
	/*! Проверяется, что данный многочлен больше многочлена polyRight 
		с другим типом мономиального порядка. */
	template<class _O1>
	bool operator>(const MP<_n, _O1>& polyRight) const
	{	
		return CompareNC(polyRight) > 0;
	}

	//! Не меньше монома?
	/*! Проверяется, что данный многочлен не меньше 
		многочлена-монома mRight.*/
	bool operator>=(const MM<_n>& mRight) const
	{	
		return !IsEmpty() && _order.Compare(*begin(), mRight) >= 0;
	}

	//! Не меньше константы?
	/*! Проверяется, что данный многочлен не меньше
		многочлена-константы cRight.*/
	bool operator>=(bool cRight) const
	{	
		return cRight ? !IsEmpty() : true;
	}

	//! Не меньше многочлена?
	/*! Проверяется, что данный многочлен не меньше многочлена polyRight 
		с тем же мономиальным порядком. */
	bool operator>=(const MP& polyRight) const
	{	
		// порядки совпадают по параметрам?
		if (_order == polyRight.GetOrder())
			// да, эффективное сравнение
			return Compare(polyRight) >= 0;
		else
			// нет, неэффективное сравнение
			return CompareNC(polyRight) >= 0;
	}

	//! Не меньше многочлена?
	/*! Проверяется, что данный многочлен не меньше многочлена polyRight 
		с другим мономиальным порядком. */
	template<class _O1>
	bool operator>=(const MP<_n, _O1>& polyRight) const
	{	
		return CompareNC(polyRight) >= 0;
	}

// конструкторы
public:
	//! Конструктор константы
	/*! Создается константный многочлен (нулевой по умолчанию). */
	MP(bool cRight = 0)
	{
		if (cRight) push_back(MM<_n>());
	}
	
	//! Конструктор монома
	/*! Создается многочлен из одного монома mRight. */
	template<size_t _m>
	MP(const MM<_m>& mRight)
	{	
		push_back(mRight);
	}

	//! Конструктор порядка
	/*! Создается нулевой многочлен с порядком oRight. */
	MP(const _O& oRight) : _order(oRight) {}

	//! Конструктор копирования
	/*! Создается копия многочлена polyRight. */
	MP(const MP& polyRight) : 
		std::list<MM<_n>>(polyRight)
	{
		SetOrder(polyRight.GetOrder());
	}

	//! Конструктор перемещения
	/*! Выполняется захват временного многочлена polyRight. */
	MP(MP&& polyRight) : 
		std::list<MM<_n>>(std::move(polyRight))
	{
		SetOrder(polyRight.GetOrder());
	}

	//! Конструктор копирования
	/*! Создается копия многочлена polyRight с другим мономиальным
		порядком. */
	template<class _O1>
	MP(const MP<_n, _O1>& polyRight) : 
		std::list<MM<_n>>(polyRight)
	{	
		Normalize();
	}

	//! Конструктор копирования
	/*! Создается копия многочлена polyRight с другим числом переменных. */
	template<size_t _m, class _O1> 
	MP(const MP<_m, _O1>& polyRight) 
	{	
		typename MP<_m, _O1>::const_iterator iter;
		for (iter = polyRight.begin(); iter != polyRight.end(); ++iter)
			SymDiff(MM<_n>(*iter));
	}
};

//! Сложение
/*! Складываются константа cLeft и моном mRight. */
template<size_t _n> inline auto 
operator+(bool cLeft, const MM<_n>& mRight)
{
	MP<_n> poly(cLeft);
	poly += mRight;
	return poly;
}

//! Сложение
/*! Складываются моном mLeft и константа cRight. */
template<size_t _n> inline auto 
operator+(const MM<_n>& mLeft, bool cRight)
{
	MP<_n> p(mLeft);
	p += cRight;
	return p;
}

//! Сложение
/*! Складываются мономы mLeft и mRight. */
template<size_t _n, size_t _m> inline auto
operator+(const MM<_n>& mLeft, const MM<_m>& mRight)
{
	MP<std::max(_n, _m)> poly(mLeft);
	poly += MM<std::max(_n, _m)>(mRight);
	return poly;
}

//! Сложение
/*! Складываются многочлен polyLeft и константа сRight. */
template<size_t _n, class _O> inline auto 
operator+(const MP<_n, _O>& polyLeft, bool cRight)
{
	MP<_n, _O> poly(polyLeft);
	poly += cRight;
	return poly;
}

template<size_t _n, class _O> inline auto
operator+(MP<_n, _O>&& polyLeft, bool cRight)
{
	MP<_n, _O> poly(std::move(polyLeft));
	poly += cRight;
	return poly;
}

//! Сложение
/*! Складываются константа cLeft и многочлен polyRight. */
template<size_t _n, class _O> inline auto 
operator+(bool cLeft, const MP<_n, _O>& polyRight)
{
	MP<_n, _O> poly(polyRight);
	poly += cLeft;
	return poly;
}

template<size_t _n, class _O> inline auto
operator+(bool cLeft, MP<_n, _O>&& polyRight)
{
	MP<_n, _O> poly(std::move(polyRight));
	poly += cLeft;
	return poly;
}
	
//! Сложение
/*! Складываются многочлен polyLeft и моном mRight. */
template<size_t _n, class _O> inline auto 
operator+(const MP<_n, _O>& polyLeft, const MM<_n>& mRight)
{
	MP<_n, _O> poly(polyLeft);
	poly += mRight;
	return poly;
}

template<size_t _n, class _O> inline auto
operator+(MP<_n, _O>&& polyLeft, const MM<_n>& mRight)
{
	MP<_n, _O> poly(std::move(polyLeft));
	poly += mRight;
	return poly;
}

//! Сложение
/*! Складываются моном mLeft и многочлен polyRight. */
template<size_t _n, class _O> inline auto 
operator+(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	MP<_n, _O> poly(polyRight);
	poly += mLeft;
	return poly;
}

template<size_t _n, class _O> inline auto
operator+(const MM<_n>& mLeft, MP<_n, _O>&& polyRight)
{
	MP<_n, _O> poly(std::move(polyRight));
	poly += mLeft;
	return poly;
}

//! Сложение
/*! Складываются многочлены polyLeft и polyRight. */
template<size_t _n, class _O1, class _O2> inline auto 
operator+(const MP<_n, _O1>& polyLeft, const MP<_n, _O2>& polyRight)
{
	MP<_n, _O1> poly(polyLeft);
	poly += polyRight;
	return poly;
}

template<size_t _n, class _O1, class _O2> inline auto
operator+(MP<_n, _O1>&& polyLeft, const MP<_n, _O2>& polyRight)
{
	MP<_n, _O1> poly(std::move(polyLeft));
	poly += polyRight;
	return poly;
}

template<size_t _n, class _O1, class _O2> inline auto
operator+(MP<_n, _O1>&& polyLeft, MP<_n, _O2>&& polyRight)
{
	MP<_n, _O1> poly(std::move(polyLeft));
	poly += polyRight;
	return poly;
}

template<size_t _n, class _O> inline auto
operator+(const MP<_n, _O>& polyLeft, MP<_n, _O>&& polyRight)
{
	MP<_n, _O> poly(std::move(polyRight));
	poly += polyLeft;
	return poly;
}

//! Умножение
/*! Умножаются многочлен polyLeft и моном mRight. */
template<size_t _n, class _O> inline auto 
operator*(const MP<_n, _O>& polyLeft, const MM<_n>& mRight)
{
	MP<_n, _O> poly(polyLeft);
	poly *= mRight;
	return poly;
}

template<size_t _n, class _O> inline auto
operator*(MP<_n, _O>&& polyLeft, const MM<_n>& mRight)
{
	MP<_n, _O> poly(std::move(polyLeft));
	poly *= mRight;
	return poly;
}

//! Умножение
/*! Умножаются моном mLeft и многочлен polyRight. */
template<size_t _n, class _O> inline auto 
operator*(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	MP<_n, _O> poly(polyRight);
	poly *= mLeft;
	return poly;
}

template<size_t _n, class _O> inline auto
operator*(const MM<_n>& mLeft, MP<_n, _O>&& polyRight)
{
	MP<_n, _O> poly(std::move(polyRight));
	poly *= mLeft;
	return poly;
}

//! Умножение
/*! Умножаются многочлены polyLeft и polyRight. */
template<size_t _n, class _O1, class _O2> inline auto 
operator*(const MP<_n, _O1>& polyLeft, const MP<_n, _O2>& polyRight)
{
	MP<_n, _O1> poly(polyLeft);
	poly *= polyRight;
	return poly;
}

template<size_t _n, class _O1, class _O2> inline auto
operator*(MP<_n, _O1>&& polyLeft, const MP<_n, _O2>& polyRight)
{
	MP<_n, _O1> poly(std::move(polyLeft));
	poly *= polyRight;
	return poly;
}

template<size_t _n, class _O1, class _O2> inline auto
operator*(MP<_n, _O1>&& polyLeft, MP<_n, _O2>&& polyRight)
{
	MP<_n, _O1> poly(std::move(polyLeft));
	poly *= polyRight;
	return poly;
}

template<size_t _n, class _O> inline auto
operator*(const MP<_n, _O>& polyLeft, MP<_n, _O>&& polyRight)
{
	MP<_n, _O> poly(std::move(polyRight));
	poly *= polyLeft;
	return poly;
}

//! Частное от деления
/*! Определяется частное от деления многочлена polyLeft на моном mRight. */
template<size_t _n, class _O> inline auto 
operator/(const MP<_n, _O>& polyLeft, const MM<_n>& mRight)
{
	MP<_n, _O> poly(polyLeft);
	poly /= mRight;
	return poly;
}

template<size_t _n, class _O> inline auto
operator/(MP<_n, _O>&& polyLeft, const MM<_n>& mRight)
{
	MP<_n, _O> poly(std::move(polyLeft));
	poly /= mRight;
	return poly;
}

//! Частное от деления
/*! Определяется частное от деления монома mLeft на многочлен polyRight.*/
template<size_t _n, class _O> inline auto 
operator/(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	MP<_n, _O> poly(mLeft);
	poly /= polyRight;
	return poly;
}

//! Частное от деления
/*! Определяется частное от деления многочлена polyLeft на polyRight. */
template<size_t _n, class _O1, class _O2> inline auto 
operator/(const MP<_n, _O1>& polyLeft, const MP<_n, _O2>& polyRight)
{
	MP<_n, _O1> poly(polyLeft);
	poly /= polyRight;
	return poly;
}

template<size_t _n, class _O1, class _O2> inline auto
operator/(MP<_n, _O1>&& polyLeft, const MP<_n, _O2>& polyRight)
{
	MP<_n, _O1> poly(std::move(polyLeft));
	poly /= polyRight;
	return poly;
}

template<size_t _n, class _O1, class _O2> inline auto
operator/(MP<_n, _O1>&& polyLeft, MP<_n, _O2>&& polyRight)
{
	MP<_n, _O1> poly(std::move(polyLeft));
	poly /= polyRight;
	return poly;
}

//! Признак делимости
/*! Проверяется, что mLeft делит все мономы polyRight. */
template<size_t _n, class _O> inline bool
operator|(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	return polyRight.IsDivisibleBy(mLeft);
}

//! Остаток от деления
/*! Определяется остаток от деления многочлена polyLeft на моном mRight. */
template<size_t _n, class _O> inline auto 
operator%(const MP<_n, _O>& polyLeft, const MM<_n>& mRight)
{
	MP<_n, _O> poly(polyLeft);
	poly %= mRight;
	return poly;
}

template<size_t _n, class _O> inline auto
operator%(const MP<_n, _O>&& polyLeft, const MM<_n>& mRight)
{
	MP<_n, _O> poly(std::move(polyLeft));
	poly %= mRight;
	return poly;
}

//! Остаток от деления
/*! Определяется остаток от деления монома mLeft 
	на многочлен polyRight.*/
template<size_t _n, class _O> inline auto
operator%(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	MP<_n, _O> poly(mLeft);
	poly %= polyRight;
	return poly;
}

//! Остаток от деления
/*! Определяется остаток от деления многочлена polyLeft на polyRight. */
template<size_t _n, class _O1, class _O2> inline auto 
operator%(const MP<_n, _O1>& polyLeft, const MP<_n, _O2>& polyRight)
{
	MP<_n, _O1> poly(polyLeft);
	poly %= polyRight;
	return poly;
}

template<size_t _n, class _O1, class _O2> inline auto
operator%(MP<_n, _O1>&& polyLeft, const MP<_n, _O2>& polyRight)
{
	MP<_n, _O1> poly(std::move(polyLeft));
	poly %= polyRight;
	return poly;
}

template<size_t _n, class _O1, class _O2> inline auto
operator%(MP<_n, _O1>&& polyLeft, MP<_n, _O2>&& polyRight)
{
	MP<_n, _O1> poly(std::move(polyLeft));
	poly %= polyRight;
	return poly;
}

//! Остаток от деления
/*! Определяется остаток от деления многочлена polyLeft на согласованную 
	систему iRight. */
template<size_t _n, class _O> inline auto 
operator%(const MP<_n, _O>& polyLeft, const Ideal<_n, _O>& iRight)
{
	MP<_n, _O> poly(polyLeft);
	poly %= iRight;
	return poly;
}

template<size_t _n, class _O> inline auto
operator%(MP<_n, _O>&& polyLeft, const Ideal<_n, _O>& iRight)
{
	MP<_n, _O> poly(std::move(polyLeft));
	poly %= iRight;
	return poly;
}

//! Равенство
/*! Проверяется равенство монома mLeft и многочлена polyRight. */
template<size_t _n, class _O> inline bool 
operator==(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	return polyRight == mLeft;
}

//! Равенство
/*! Проверяется равенство константы cLeft и многочлена polyRight. */
template<size_t _n, class _O> inline bool 
operator==(bool cLeft, const MP<_n, _O>& polyRight)
{
	return polyRight == cLeft;
}

//! Неравенство
/*! Проверяется неравенство монома mLeft и многочлена polyRight. */
template<size_t _n, class _O> inline bool 
operator!=(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	return polyRight != mLeft;
}

//! Неравенство
/*! Проверяется неравенство константы cLeft и многочлена polyRight. */
template<size_t _n> inline bool 
operator!=(bool cLeft, const MP<_n>& polyRight)
{
	return polyRight != cLeft;
}

//! Меньше
/*! Проверяется, что многочлен-моном mLeft меньше многочлена polyRight.*/
template<size_t _n, class _O> inline bool 
operator<(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	return polyRight > mLeft;
}

//! Меньше
/*! Проверяется, что многочлен-константа mLeft меньше 
	многочлена polyRight.*/
template<size_t _n, class _O> inline bool 
operator<(bool cLeft, const MP<_n, _O>& polyRight)
{
	return polyRight > cLeft;
}

//! Не больше
/*! Проверяется, что многочлен-моном mLeft не больше 
	многочлена polyRight.*/
template<size_t _n, class _O> inline bool 
operator<=(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	return polyRight >= mLeft;
}

//! Не больше
/*! Проверяется, что многочлен-константа mLeft не больше
	многочлена polyRight.*/
template<size_t _n, class _O> inline bool 
operator<=(bool cLeft, const MP<_n, _O>& polyRight)
{
	return polyRight >= cLeft;
}

//! Больше
/*! Проверяется, что многочлен-моном mLeft больше 
	многочлена polyRight.*/
template<size_t _n, class _O> inline bool 
operator>(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	return polyRight < mLeft;
}

//! Больше
/*! Проверяется, что многочлен-константа mLeft больше
	многочлена polyRight.*/
template<size_t _n, class _O> inline bool 
operator>(bool cLeft, const MP<_n, _O>& polyRight)
{
	return polyRight < cLeft;
}

//! Не меньше
/*! Проверяется, что многочлен-моном mLeft не меньше 
	многочлена polyRight.*/
template<size_t _n, class _O> inline bool 
operator>=(const MM<_n>& mLeft, const MP<_n, _O>& polyRight)
{
	return polyRight <= mLeft;
}

//! Не меньше
/*! Проверяется, что многочлен-константа mLeft не меньше
	многочлена polyRight.*/
template<size_t _n, class _O> inline bool 
operator>=(bool cLeft, const MP<_n, _O>& polyRight)
{
	return polyRight <= cLeft;
}

//! Вывод в поток
/*! Многочлен polyRight выводится в поток os. */
template<class _Char, class _Traits, size_t _n, class _O> inline 
std::basic_ostream<_Char, _Traits>& 
operator<<(std::basic_ostream<_Char, _Traits>& os, const MP<_n, _O>& polyRight)
{
	bool waitfirst = true;
	typename MP<_n, _O>::const_iterator iter = polyRight.begin();
	for (; iter != polyRight.end(); ++iter)
	{
		if (!waitfirst) os << " + ";
		os << *iter;
		waitfirst = false;
	}
	return os << (waitfirst ? "0" : "");
}

//! Ввод из потока
/*! Многочлен polyRight читается из потока is. 
	\par Допустимый ввод представляет собой набор из лексем-мономов,
	включая лексему "0". Лексемы должны быть разделены знаками +. 
	Между лексемами и знаками + допускается наличие произвольного 
	числа пробелов, знаков табуляции и других пустых разделителей 
	из набора " \n\r\t\v". Допускается повтор лексем.
	\par Чтение многочлена прекращается, если 
	1) достигнут конец файла или встретилась неподходящая лексема,
	2) при разборе лексем возникла ошибка. 
	Если не прочитано ни одной лексемы, либо возникла ошибка при разборе
	лексем, то в состоянии потока будет установлен флаг 
	ios_base::failbit, без снятия которого дальнейшее чтение 
	из потока невозможно. 
	\par По окончании ввода многочлен polyRight будет содержать 
	упрощенную сумму прочитанных мономов. */
template<class _Char, class _Traits, size_t _n, class _O> inline
std::basic_istream<_Char, _Traits>& 
operator>>(std::basic_istream<_Char, _Traits>& is, MP<_n, _O>& polyRight)
{	
	// предварительно обнуляем многочлен
	polyRight = 0;
	// признак изменения
	bool changed = false;
	// захватываем поток и пропускаем пустые символы
	typename std::basic_istream<_Char, _Traits>::sentry s(is);
	// захват выполнен? 
	if (s)
	{
		// флаг ожидания знака +
		bool waitforplus = false;
		// читаем символы
		while (true)
		{
			// читаем символ из потока
			auto c1 = is.rdbuf()->sbumpc();
			// конец файла?
			if (_Traits::eq_int_type(c1, _Traits::eof()))
			{
				is.setstate(std::ios_base::eofbit);
				// ничего не прочитано?
				if (!changed) 
					is.setstate(std::ios_base::failbit);
				break;
			}
			// определяем символ
			char c = is.narrow(_Traits::to_char_type(c1), '*');
			// разделитель?
			if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
				continue;
			// ожидаем +?
			if (waitforplus)
			{
				if (c != '+')
				{
					is.rdbuf()->sputbackc(_Traits::to_char_type(c1));
					break;
				}
				waitforplus = false;
				continue;
			}
			// лексема 0?
			if (c == '0')
			{
				waitforplus = changed = true;
				continue;
			}
			// возвращаем символ обратно и читаем моном
			is.rdbuf()->sputbackc(_Traits::to_char_type(c1));
			MM<_n> m;
			is >> m;
			// ошибка чтения?
			if (!is.good())
			{
				is.setstate(std::ios_base::failbit);
				break;
			}
			// добавить моном
			polyRight += m;
			waitforplus = changed = true;
		}
	}
	// чтение требовалось, но не удалось?
	else
		is.setstate(std::ios_base::failbit);
	return is;
}

} // namespace GF2

#endif // __GF2_MP
