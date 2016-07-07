/*
*******************************************************************************
\file ideal.h
\brief Ideals in GF(2)[x0,x1,...]
\project GF2 [GF(2) algebra library]
\author (С) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2004.01.01
\version 2016.07.07
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file ideal.h
\brief Идеалы в GF(2)[x0,x1,...]

Модуль содержит описание и реализацию класса Ideal,
поддерживающего манипуляции с идеалами в кольце многочленов 
от нескольких переменных над полем из двух элементов.
*******************************************************************************
*/

#ifndef __GF2_IDEAL
#define __GF2_IDEAL

#include "gf2/defs.h"
#include "gf2/mpoly.h"
#include "gf2/zz.h"
#include <list>
#include <iostream>

namespace GF2{

/*!
*******************************************************************************
Класс Ideal

-#	Поддерживает операции с идеалом I кольца R= GF(2)[x_0,\ldots,x_{n-1}]. 
	Идеал задается системой образующих и мономиальным порядком.
	Явно заданные образующие -- это, так называемые, мультилинейные
	многочлены, которые поддерживаются классом MPoly.
	Неявные образующие -- уравнения поля вида x_i^2 + x_i.
-#	Класс порожден от списка std::list. Элементами списка являются различные 
	мультилинейные многочлены системы. При управлении списками предполагается,
	что многочлены не повторяются, не являются нулевыми и упорядочены 
	по возрастанию в порядке _O.
*******************************************************************************
*/

template<size_t _n, class _O> class Ideal : public std::list<MPoly<_n, _O> >
{
public:
	using typename std::list<MPoly<_n, _O> >::iterator;
	using typename std::list<MPoly<_n, _O> >::const_iterator;
	using typename std::list<MPoly<_n, _O> >::const_reverse_iterator;
	using std::list<MPoly<_n, _O> >::begin;
	using std::list<MPoly<_n, _O> >::clear;
	using std::list<MPoly<_n, _O> >::end;
	using std::list<MPoly<_n, _O> >::erase;
	using std::list<MPoly<_n, _O> >::insert;
	using std::list<MPoly<_n, _O> >::rbegin;
	using std::list<MPoly<_n, _O> >::rend;
	using std::list<MPoly<_n, _O> >::size;
	using std::list<MPoly<_n, _O> >::sort;
	using std::list<MPoly<_n, _O> >::splice;
	using std::list<MPoly<_n, _O> >::swap;
// число переменных
public:
	//! раскрытие числа переменных _n
	enum {n = _n};

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
	/*! Устанавливается мономиальный порядок. 
		\remark Имеет смысл устанавливать только параметризуемые 
		мономиальные порядки, поскольку все непараметризуемые порядки типа 
		_O обязательно совпадают. */
	void SetOrder(const _O& oRight)
	{
		if (!(_order == oRight))
		{
			for (iterator iter = begin(); iter != end(); ++iter)
				iter->SetOrder(oRight);
			_order = oRight;
		}
	}

	//! Согласованность
	/*! Проверяется, что мономиальный порядок совпадает с порядком
		многочлена polyRight. */
	bool IsConsistent(const MPoly<_n, _O>& polyRight) const
	{
		return _order == polyRight.GetOrder();
	}

	//! Согласованность
	/*! Проверяется, что мономиальный порядок совпадает с порядком
		идеала iRight. */
	bool IsConsistent(const Ideal& iRight) const
	{
		return _order == iRight.GetOrder();
	}

// управление списком многочленов
public:
	//! Нормализация
	/*! Выполняется сортировка многочленов по возрастанию и удаление 
		нулевых и одинаковых многочленов. */
	void Normalize()
	{	
		// сортировка
		sort();
		// удаление повторов
		iterator iter = begin(), iterNext;
		while (iter != end() && ++(iterNext = iter) != end())
			if (*iter == *iterNext) iter = erase(iter);
			else iter = iterNext; 
		// удаление нулевого многочлена
		if (size() && *begin() == 0) erase(begin());
	}

	//! Нормализован?
	/*! Проверяется, что в списке нет нулевых мнгочленов,
		многочлены не повторяются и отсортированы по возрастанию.*/
	bool IsNormalized() const
	{	
		const_iterator iter = begin(), iterNext;
		if (iter == end()) return true;
		else if (*iter == 0) return false;
		while (iter != end() && ++(iterNext = iter) != end())
			if (*iter >= *iterNext)
				return false;
			else iter = iterNext;
		return true;
	}

	//! Поиск многочлена
	/*! Возвращется признак вхождения однотипного многочлена polyRight 
		в систему. В pos возвращается первая позиция в списке, 
		вставка в которую polyRight не нарушит порядок. */
	bool Find(const MPoly<_n, _O>& polyRight, iterator& pos)
	{
		if (IsConsistent(polyRight))
		{
			pos = std::lower_bound(begin(), end(), polyRight);
			return pos != end() && *pos == polyRight;
		}
		// сравнение несогласованных многочленов неэффективно,
		// поэтому сразу меняем порядок
		MPoly<_n, _O> poly(_order);
		pos = std::lower_bound(begin(), end(), poly = polyRight);
		return pos != end() && *pos == poly;
	}

	//! Поиск многочлена
	/*! Возвращется признак вхождения однотипного многочлена polyRight 
		в систему. В pos возвращается первая позиция в списке, 
		вставка в которую polyRight не нарушит порядок. */
	bool Find(const MPoly<_n, _O>& polyRight, const_iterator& pos) const
	{
		if (IsConsistent(polyRight))
		{
			pos = std::lower_bound(begin(), end(), polyRight);
			return pos != end() && *pos == polyRight;
		}
		MPoly<_n, _O> poly(_order);
		pos = std::lower_bound(begin(), end(), poly = polyRight);
		return pos != end() && *pos == poly;
	}

	//! Поиск многочлена
	/*! Возвращется признак вхождения в систему многочлена polyRight 
		другого типа и возможно с другим числом переменных. 
		В pos возвращается первая позиция в списке, 
		вставка в которую polyRight не нарушит порядок. */
	template<size_t _m, class _O1>
	bool Find(const MPoly<_m, _O1>& polyRight, iterator& pos)
	{
		MPoly<_n, _O> poly(_order);
		pos = std::lower_bound(begin(), end(), poly = polyRight);
		return pos != end() && *pos == poly;
	}

	//! Поиск многочлена
	/*! Возвращается признак вхождения в систему многочлена polyRight 
		другого типа и возможно с другим числом переменных. 
		В pos возвращается первая позиция в списке, 
		вставка в которую polyRight не нарушит порядок. */
	template<size_t _m, class _O1>
	bool Find(const MPoly<_m, _O1>& polyRight, const_iterator& pos) const
	{
		MPoly<_n, _O> poly(_order);
		pos = std::lower_bound(begin(), end(), poly = polyRight);
		return pos != end() && *pos == poly;
	}

	//! Содержит многочлен?
	/*! Проверка принадлежности системе многочлена polyRight. */
	template<size_t _m, class _O1>
	bool IsContain(const MPoly<_m, _O1>& polyRight) const
	{	
		return Find(polyRight, const_iterator());
	}

	//! Добавление многочлена
	/*! В систему добавляется ненулевой многочлен polyRight. 
		\return позиция polyRight в списке. */
	template<size_t _m, class _O1>
	iterator Insert(const MPoly<_m, _O1>& polyRight)
	{	
		assert(!polyRight.IsEmpty());
		iterator pos;
		if (!Find(polyRight, pos))
			*(pos = insert(pos, MPoly<_n, _O>(_order))) = polyRight;
		return pos;
	}

	//! Добавление системы
	/*! В систему добавляются многочлены другой системы iRight. */
	template<size_t _m, class _O1>
	void Insert(const Ideal<_m, _O1>& iRight)
	{	
		// другая система?
		assert(static_cast<const void*>(this) != static_cast<const void*>(&iRight));
		typename Ideal<_m, _O1>::const_iterator iter;
		for (iter = iRight.begin(); iter != iRight.end(); ++iter)
			Insert(*iter);
	}

	//! Удаление многочлена
	/*! Из системы удаляется многочлен polyRight. */
	template<size_t _m, class _O1>
	void Remove(const MPoly<_m, _O1>& polyRight)
	{	
		iterator pos;
		if (Find(polyRight, pos))
			erase(pos);
	}

	//! Удаление системы
	/*! Из системы удаляются все многочлены другой системы iRight. */
	template<size_t _m, class _O1>
	void Remove(const Ideal<_m, _O1>& iRight)
	{	
		// другая система?
		assert(static_cast<void*>(this) != static_cast<void*>(&iRight));
		typename Ideal<_m, _O1>::const_iterator iter;
		for (iter = iRight.begin(); iter != iRight.end(); ++iter)
			Remove(*iter);
	}

	//! Удаление многочленов определенной степени
	/*! Из системы удаляются все многочлены степени d, 
		для которых predicate(d, nDeg) == true. 
		_Pr -- класс предикатов, порожденный от 
		std::binary_function<int, int, bool>. 
		\example RemoveDeg(3, std::greater_equal<int>()). */
	template<class _Pr>
	void RemoveDeg(int nDeg, _Pr predicate)
	{	
		iterator iter;
		for (iterator iter = begin(); iter != end();)
			if (predicate(iter->Deg(), nDeg))
				iter = erase(iter);
			else ++iter;
	}

	//! Удаление многочлена в позиции
	/*! Из системы удаляется многочлен в позиции pos. 
		\return позиция следующего многочлена системы. */
	iterator RemoveAt(iterator pos)
	{	
		return erase(pos);
	}

	//! Перемещение многочлена
	/*! Многочлен в позиции pos перемещается в системе так, 
		что система остается нормализованной. 
		\pre многочлен должен быть ненулевым.
		\return позиция, перед которой был вставлен многочлен 
		(pos, если многочлен не переместился).
		\remark Метод следует вызывать после каждого 
		изменения отдельных многочленов системы. */
	iterator Move(iterator pos)
	{
		assert(*pos != 0);
		iterator where, after = pos;
		where = std::lower_bound(begin(), pos, *pos), ++after;
		// перемещаем в младшую сторону?
		if (where != pos)
			splice(where, *this, pos, after);
		else
		{
			where = std::lower_bound(after, end(), *pos);
			// перемещаем в старшую сторону?
			if (where != after)
				splice(where, *this, pos, after);
			// не перемещаем
			where = pos;
		}
		return where;
	}

	//! Обмен
	/*! Производится обмен списком многочленов с согласованной системой
		iRight. */
	void Swap(Ideal& iRight)
	{
		assert(IsConsistent(iRight));
		swap(iRight);
	}

	//! Заем многочлена
	/*! Производится заем из согласованной системы iRight 
		многочлена в позиции pos. 
		\return позиция занятого многочлена.
		\remark Даже если многочлен уже присутствует в системе,
		он все равно удаляется из iRight. */
	iterator Splice(Ideal& iRight, iterator pos)
	{
		assert(IsConsistent(iRight));
		iterator where;
		if (Find(*pos, where))
			iRight.RemoveAt(pos);
		else
			splice(where, iRight, pos), --where;
		return where;
	}

	//! Очистка
	/*! Удаляются все многочлены. */
	void SetEmpty()
	{	
		clear();
	}

	//! Пустая система?
	/*! Проверяется, что система пуста. */
	bool IsEmpty() const
	{	
		return Size() == 0;
	}

	//! Сортировка
	/*! Многочлены системы сортируются по возрастанию. */
	void Sort()
	{	
		sort();
	}

	//! Число многочленов
	/*! Определяется число многочленов в системе. */
	size_t Size() const
	{	
		return (size_t)size();
	}

	//! Минимальная степень многочленов
	/*! Определяется минимальная степень многочленов системы 
		(-1 для пустой системы). */
	int MinDeg() const
	{	
		int mindeg = IsEmpty() ? -1 : begin()->Deg();
		int cur;
		for (const_iterator iter = ++begin(); iter != end(); ++iter)
			if ((cur = iter->Deg()) < mindeg) mindeg = cur;
		return mindeg;
	}

	//! Максимальная степень многочленов
	/*! Определяется максимальная степень многочленов системы 
		(-1 для пустой системы). */
	int MaxDeg() const
	{	
		int maxdeg = IsEmpty() ? -1 : begin()->Deg();
		int cur;
		for (const_iterator iter = ++begin(); iter != end(); ++iter)
			if ((cur = iter->Deg()) > maxdeg) maxdeg = cur;
		return maxdeg;
	}

	//! Число многочленов определенной степени
	/*! Определяется многочлены степени d, где
		predicate(d, nDeg) == true. 
		_Pr -- класс предикатов, порожденный от 
		std::binary_function<int, int, bool>. 
		\example CountDeg(3, std::greater_equal<int>()). */
	template<class _Pr>
	size_t CountDeg(int nDeg, _Pr predicate) const
	{	
		size_t nCount = 0;
		for (const_iterator iter = begin(); iter != end(); ++iter)
			if (predicate(iter->Deg(), nDeg)) nCount++;
		return nCount;
	}

// управление мономами
public:
	//! Сбор переменных
	/*! Возвращается моном, который фиксирует вхождения всех переменных 
		системы. */		
	Monom<_n> GatherVars() const
	{	
		Monom<_n> vars;
		for (const_iterator iter = begin(); iter != end(); ++iter)
		{
			typename MPoly<_n>::const_iterator iterPoly = iter->begin();
			for (; iterPoly != iter->end(); ++iterPoly)
				vars *= *iterPoly;
		}
		return vars;
	}

	//! Сбор мономов
	/*! Возвращаемый по ссылке согласованный многочлен polyMons содержит 
		все мономы многочленов системы. 
		\return количество мономов. */
	size_t GatherMons(MPoly<_n, _O>& polyMons) const
	{	
		// согласованный?
		assert(IsConsistent(polyMons));
		polyMons.SetEmpty();
		for (const_iterator iter = begin(); iter != end(); ++iter)
			polyMons.Union(*iter);
		return polyMons.Size();
	}

	//! Сбор старших мономов
	/*! Возвращаемый по ссылке согласованный многочлен polyMons содержит 
		все старшие мономы многочленов системы. 
		\return количество мономов.	*/
	size_t GatherLMons(MPoly<_n, _O>& polyMons) const
	{	
		// согласованный?
		assert(IsConsistent(polyMons));
		polyMons.SetEmpty();
		for (const_iterator iter = begin(); iter != end(); ++iter)
			polyMons.Union(iter->LM());
		return polyMons.Size();
	}

	//! Сбор минимальных старших мономов
	/*! Возвращаемый по ссылке согласованный многочлен polyMons содержит 
		минимальные (такие, которые не делятся на другие) 
		старшие мономы многочленов системы. 
		\return количество мономов.	*/
	size_t GatherMinLMons(MPoly<_n, _O>& polyMons) const
	{
		GatherLMons(polyMons);
		typename MPoly<_n, _O>::iterator iter;
		// от старших мономов к младшим
		for (iter = polyMons.begin(); iter != polyMons.end();)
		{
			// от младших мономов к старшим
			typename MPoly<_n, _O>::iterator iter1 = --polyMons.end();
			for (; iter1 != iter; --iter1)
				if (*iter1 | *iter)
					break;
			if (iter1 == iter) ++iter;
			else iter = polyMons.erase(iter);
		}
		return polyMons.Size();
	}

	//! Старший моном
	/*! Возвращается самый старший моном многочленов системы. 
		\pre система не может быть пустой. */
	const Monom<_n>& LM() const
	{	
		// система не пуста?
		assert(!IsEmpty());
		const_iterator iterLM = begin(), iter = iterLM;
		for (++iter; iter != end(); ++iter)
			if (_order.Compare(iter->LM(), iterLM->LM()) > 0) 
				iterLM = iter;
		return iterLM->LM();
	}

	//! Упаковка
	/*! Переменные, которые входят в mMask, заменяются на переменные x0, x1,...
		Остальные переменные исключаются. */
	void Pack(const Monom<_n>& mMask)
	{
		for (iterator iter = begin(); iter != end(); ++iter)
			iter->Pack(mMask);
		Normalize();
	}

	//! Распаковка
	/*! Переменные x0, x1,... заменяются на переменные, которые входят в mMask. */
	void Unpack(const Monom<_n>& mMask)
	{
		for (iterator iter = begin(); iter != end(); ++iter)
			iter->Unpack(mMask);
		Normalize();
	}


// операции
public:
	//! Присваивание системы
	/*! Присваивание слову значения-системы iRight. */
	template<size_t _m, class _O1>
	Ideal& operator=(const Ideal<_m, _O1>& iRight)
	{	
		if (static_cast<void*>(this) != static_cast<void*>(&iRight))
			SetEmpty(), Insert(iRight);
		return *this;
	}

	//! Равенство системе
	/*! Проверяется равенство системе iRight. */
	template<size_t _m, class _O1>
	bool operator==(const Ideal<_m, _O1>& iRight) const
	{	
		if (Size() != iRight.Size()) 
			return false;
		typename Ideal<_m, _O1>::const_iterator iter;
		for (iter = iRight.begin(); iter != iRight.end(); ++iter)
			if (!IsContain(*iter)) 
				return false;
		return true;
	}

	//! Неравенство системе
	/*! Проверяется неравенство системе iRight. */
	template<size_t _m, class _O1>
	bool operator!=(const Ideal<_m, _O1>& iRight) const
	{	
		if (Size() != iRight.Size()) 
			return true;
		typename Ideal<_m, _O1>::const_iterator iter;
		for (iter = iRight.begin(); iter != iRight.end(); ++iter)
			if (!IsContain(*iter)) 
				return true;
		return false;
	}

// приведение
public:
	//! Приведение многочлена
	/*! Определяется нормальная форма согласованного многочлена polyRight,
		т.е. polyRight заменяется остатками от деления на последовательные 
		многочлены системы. Деления прекращаются, когда ни один из мономов
		polyRight не делится ни на один из старших мономов системы.
		\remark Нормальная форма, вообще говоря, 
		определяется очередностью делений и определена однозначно, 
		только если система является базисом Гребнера.
		\return Признак того, что выходной polyRight отличается от входного. */
	bool ReduceClassic(MPoly<_n, _O>& polyRight) const
	{	
		assert(IsConsistent(polyRight));
		bool changed = false;
		MPoly<_n, _O> poly(_order);
		// двигаемся от старших мономов polyRight к младшим
		typename MPoly<_n, _O>::iterator iterMon = polyRight.begin();
		while (iterMon != polyRight.end())
		{
			// двигаемся от младших многочленов системы к старшим
			const_iterator iterPoly = begin();
			for (; iterPoly != end(); ++iterPoly)
			{
				// прекращаем заведомо нерезультативные итерации
				if (_order.Compare(iterPoly->LM(), *iterMon) > 0)
				{
					iterPoly = end();
					break;
				}
				// делим при возможности
				else if (iterPoly->LM() | *iterMon)
				{
					changed = true;
					(poly = *iterPoly) *= *iterMon / iterPoly->LM();
					polyRight.SymDiffSplice(poly);
					iterMon = polyRight.begin();
					break;
				}
			}
			if (iterPoly == end()) ++iterMon;
		}
		return changed;
	}

	//! Минимизация многочлена
	/*! Cогласованный многочлен polyRight заменяется остатками
		от деления на последовательные многочлены системы. 
		Деления прекращаются, когда старший моном polyRight 
		не делится ни на один из старших мономов системы.
		\return Признак того, что выходной polyRight отличается от входного. */
	bool MinimizeClassic(MPoly<_n, _O>& polyRight) const
	{	
		assert(IsConsistent(polyRight));
		bool changed = false;
		MPoly<_n, _O> poly(_order);
		const_iterator iterPoly = begin();
		while (iterPoly != end() && polyRight.Size())
		{
			// двигаемся от младших многочленов системы к старшим
			for (; iterPoly != end(); ++iterPoly)
				// прекращаем заведомо нерезультативные итерации
				if (_order.Compare(iterPoly->LM(), polyRight.LM()) > 0)
				{
					iterPoly = end();
					break;
				}
				// делим при возможности
				else if (iterPoly->LM() | polyRight.LM())
				{
					changed = true;
					(poly = *iterPoly) *= polyRight.LM() / iterPoly->LM();
					polyRight.SymDiffSplice(poly);
					iterPoly = begin();
					break;
				}
		}
		return changed;
	}

	//! Приведение многочлена
	/*! Определяется нормальная форма согласованного многочлена polyRight,
		т.е. polyRight заменяется остатками от деления на последовательные 
		многочлены системы. Деления прекращаются, когда ни один из мономов
		polyRight не делится ни на один из старших мономов системы.
		При вычислениях используется структура geobucket.
		\return Признак того, что выходной polyRight отличается от входного. */
	bool Reduce(MPoly<_n, _O>& polyRight) const
	{	
		typename MPoly<_n, _O>::template Geobucket<2> gb(polyRight);
		// будем сохранять в polyRight остаток
		polyRight.SetEmpty();
		// цикл деления
		bool changed = false;
		Monom<_n> lm;
		MPoly<_n, _O> poly(_order);
		while (gb.PopLM(lm))
		{
			// двигаемся от младших многочленов системы к старшим
			const_iterator iterPoly = begin();
			for (; iterPoly != end(); ++iterPoly)
			{
				// прекращаем заведомо нерезультативные итерации
				if (_order.Compare(iterPoly->LM(), lm) > 0)
				{
					iterPoly = end();
					break;
				}
				// делимость старших мономов?
				if (iterPoly->LM() | lm)
				{
					changed = true;
					// готовим множитель
					(poly = *iterPoly).PopLM();
					// перемножаем и добавляем в geobucket
					gb.SymDiffSplice(poly *= (lm /= iterPoly->LM()));
					// все сначала
					break;
				}
			}
			// lm не делится ни на один из старших многочленов системы,
			// поэтому заносим lm в остаток
			if (iterPoly == end()) polyRight.push_back(lm);
		}
		return changed;
	}

	//! Минимизация многочлена
	/*! Cогласованный многочлен polyRight заменяется остатками
		от деления на последовательные многочлены системы. 
		Деления прекращаются, когда старший моном polyRight 
		не делится ни на один из старших мономов системы.
		При вычислениях используется структура geobucket.
		\return Признак того, что выходной polyRight отличается от входного. */
	bool Minimize(MPoly<_n, _O>& polyRight) const
	{	
		typename MPoly<_n, _O>::template Geobucket<2> gb(polyRight);
		// будем сохранять в polyRight остаток
		polyRight.SetEmpty();
		// цикл деления
		bool changed = false;
		Monom<_n> lm;
		MPoly<_n, _O> poly(_order);
		const_iterator iterPoly = begin();
		while (iterPoly != end() && gb.PopLM(lm))
		{
			// двигаемся от младших многочленов системы к старшим
			for (; iterPoly != end(); ++iterPoly)
			{
				// прекращаем заведомо нерезультативные итерации
				if (_order.Compare(iterPoly->LM(), lm) > 0)
				{
					iterPoly = end();
					break;
				}
				// делимость старших мономов?
				if (iterPoly->LM() | lm)
				{
					changed = true;
					// готовим множитель
					(poly = *iterPoly).PopLM();
					// перемножаем и добавляем в geobucket
					gb.SymDiffSplice(poly *= (lm /= iterPoly->LM()));
					// все сначала
					iterPoly = begin();
					break;
				}
			}
			// lm не делится ни на один из старших многочленов системы
			// возвращаем lm в geobucket и заканчиваем
			if (iterPoly == end()) gb.SymDiff(lm);
		}
		gb.Mount(polyRight);
		return changed;
	}

	//! Приведение многочлена системы
	/*! Определяется нормальная форма многочлена системы в позиции pos.
		\return Признак того, что в результате приведения многочлен изменился.
		\remark Многочлен в позиции pos может быть изменен.
		Для возврата к нормализованной системе следует выполнить:
		\code
			if (*pos == 0) RemoveAt(pos);
			else Move(pos);
		\endcode 
	*/
	bool ReduceClassic(iterator pos)
	{	
		bool changed = false;
		MPoly<_n, _O> poly(_order);
		typename MPoly<_n, _O>::iterator iterMon = pos->begin();
		while (iterMon != pos->end())
		{
			iterator iterPoly = begin();
			for (; iterPoly != end(); ++iterPoly)
			{
				// пропускаем многочлен в позиции pos
				if (iterPoly == pos) continue;
				// все остальное без изменений
				if (_order.Compare(iterPoly->LM(), *iterMon) > 0)
				{
					iterPoly = end();
					break;
				}
				else if (iterPoly->LM() | *iterMon)
				{
					changed = true;
					(poly = *iterPoly) *= *iterMon / iterPoly->LM();
					pos->SymDiffSplice(poly);
					iterMon = pos->begin();
					break;
				}
			}
			if (iterPoly == end()) ++iterMon;
		}
		return changed;
	}

	//! Минимизация многочлена системы
	/*! Многочлен системы в позиции pos минимизируется.
		\return Признак того, что в результате приведения многочлен
		изменился.
		\remark Многочлен в позиции pos может быть изменен.
		Для возврата к нормализованной системе следует выполнить:
		\code
			if (*pos == 0) RemoveAt(pos);
			else Move(pos);
		\endcode 
	*/
	bool MinimizeClassic(iterator pos) const
	{	
		bool changed = false;
		MPoly<_n, _O> poly(_order);
		const_iterator iterPoly = begin();
		while (iterPoly != end() && pos->Size())
		{
			for (; iterPoly != end(); ++iterPoly)
			{
				// пропускаем многочлен в позиции pos
				if (iterPoly == pos) continue;
				// все остальное без изменений
				if (_order.Compare(iterPoly->LM(), pos->LM()) > 0)
				{
					iterPoly = end();
					break;
				}
				else if (iterPoly->LM() | pos->LM())
				{
					changed = true;
					(poly = *iterPoly) *= pos->LM() / iterPoly->LM();
					pos->SymDiffSplice(poly);
					iterPoly = begin();
					break;
				}
			}
		}
		return changed;
	}

	//! Приведение многочлена системы
	/*! Определяется нормальная форма многочлена системы в позиции pos.
		При вычислениях используется структура geobucket.
		\return Признак того, что нормальная форма отличается от исходного 
		многочлена. 
		\remark Многочлен в позиции pos может быть изменен.
		Для возврата к нормализованной системе следует выполнить:
		\code
			if (*pos == 0) RemoveAt(pos);
			else Move(pos);
		\endcode
		*/
	bool Reduce(iterator pos)
	{	
		typename MPoly<_n, _O>::template Geobucket<2> gb(*pos);
		// будем сохранять в *pos остаток
		pos->SetEmpty();
		// цикл деления
		bool changed = false;
		Monom<_n> lm;
		MPoly<_n, _O> poly(_order);
		while (gb.PopLM(lm))
		{
			iterator iterPoly = begin();
			for (; iterPoly != end(); ++iterPoly)
			{
				// пропускаем многочлен в позиции pos
				if (iterPoly == pos) continue;
				// все остальное без изменений
				if (_order.Compare(iterPoly->LM(), lm) > 0)
				{
					iterPoly = end();
					break;
				}
				else if (iterPoly->LM() | lm)
				{
					changed = true;
					(poly = *iterPoly).PopLM();
					gb.SymDiffSplice(poly *= (lm /= iterPoly->LM()));
					break;
				}
			}
			if (iterPoly == end()) pos->push_back(lm);
		}
		return changed;
	}

	//! Минимизация многочлена системы
	/*! Многочлен системы в позиции pos минимизируется.
		При вычислениях используется структура geobucket.
		\return Признак того, что нормальная форма отличается от исходного 
 		многочлена. 
		\remark Многочлен в позиции pos может быть изменен.
		Для возврата к нормализованной системе следует выполнить:
		\code
			if (*pos == 0) RemoveAt(pos);
			else Move(pos);
		\endcode
		*/
	bool Minimize(iterator pos) const
	{	
		typename MPoly<_n, _O>::template Geobucket<2> gb(*pos);
		// будем сохранять в *pos остаток
		pos->SetEmpty();
		// цикл деления
		bool changed = false;
		Monom<_n> lm;
		MPoly<_n, _O> poly(_order);
		const_iterator iterPoly = begin();
		while (iterPoly != end() && gb.PopLM(lm))
		{
			// двигаемся от младших многочленов системы к старшим
			for (; iterPoly != end(); ++iterPoly)
			{
				// пропускаем многочлен в позиции pos
				if (iterPoly == pos) continue;
				// все остальное без изменений
				if (_order.Compare(iterPoly->LM(), lm) > 0)
				{
					iterPoly = end();
					break;
				}
				// делимость старших мономов?
				if (iterPoly->LM() | lm)
				{
					changed = true;
					// готовим множитель
					(poly = *iterPoly).PopLM();
					// перемножаем и добавляем в geobucket
					gb.SymDiffSplice(poly *= (lm /= iterPoly->LM()));
					// все сначала
					iterPoly = begin();
					break;
				}
			}
			// lm не делится ни на один из старших многочленов системы
			// возвращаем lm в geobucket и заканчиваем
			if (iterPoly == end()) gb.SymDiff(lm);
		}
		gb.Mount(*pos);
		return changed;
	}

	//! Самоприведение
	/*! Многочлены системы заменяются на нормальные формы.
		Нулевые формы исключаются из системы, а упрощение прекращается, 
		когда все нормальные формы совпадают с исходными многочленами. */
	Ideal& SelfReduce()
	{	
		// важно, чтобы система была нормализована
		assert(IsNormalized());
		// цикл замены на нормальные формы
		bool changed;
		do
		{
			changed = false;
			// двигаемся от старших многочленов к младшим
			for (iterator pos = end(); pos != begin();)
				// есть изменения?
				if (Reduce(--pos))
				{
					changed = true;
					Env::Trace("SelfReduce: %d polys (%d mons)", 
						Size(), pos->Size());
					// нулевая форма? исключаем
					if (*pos == 0)
						pos = RemoveAt(pos);
					// ненулевая? перемещаем
					else
						Move(pos++);
				}
		}
		while (changed);
		return *this;
	}

	//! Самоминимизация
	/*! Многочлены системы минимизируются.
		Нулевые результаты исключаются из системы, а упрощение прекращается, 
		когда старшие мономы многочленов не делятся друг на друга. */
	Ideal& SelfMinimize()
	{	
		// важно, чтобы система была нормализована
		assert(IsNormalized());
		// цикл минимизации
		bool changed;
		do
		{
			changed = false;
			// двигаемся от старших многочленов к младшим
			for (iterator pos = end(); pos != begin();)
				// есть изменения?
				if (Minimize(--pos))
				{
					changed = true;
					Env::Trace("SelfMinimize: %d polys (%d mons)", 
						Size(), pos->Size());
					// нулевая форма? исключаем
					if (*pos == 0)
						pos = RemoveAt(pos);
					// ненулевая? перемещаем
					else
						Move(pos++);
				}
		}
		while (changed);
		return *this;
	}

	//! Замена переменной
	/*! Для всех многочленов системы выполняется замена вхождений переменной 
		с номером pos на многочлен polyReplace. */
	template<class _O1>
	void Replace(size_t pos, const MPoly<_n, _O1>& polyReplace)
	{	
		for (iterator iter = begin(); iter != end(); ++iter)
			iter->Replace(pos, polyReplace);
		Normalize();
	}

	//! Замена переменной
	/*! Для всех многочленов системы выполняется замена вхождений переменной 
		с номером pos на переменную с номером posNew. */
	void Replace(size_t pos, size_t posNew)
	{	
		if (pos == posNew) return;
		for (iterator iter = begin(); iter != end(); ++iter)
			iter->Replace(pos, posNew);
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
	/*! Для всех многочленов системы выполняется замена вхождений переменной 
		с номером pos на константу val. */
	void Set(size_t pos, bool val)
	{	
		for (iterator iter = begin(); iter != end(); ++iter)
			iter->Set(pos, val);
		Normalize();
	}

//  базис Гребнера
public:
	//! Базис Гребнера?
	/*! Проверяется, что система многочленов является базисом Гребнера 
		порождаемого идеала. */
	bool IsGB() const
	{
		MPoly<_n, _O> poly(_order);
		// цикл по многочленам
		word trace = 0;
        for (const_reverse_iterator iter = rbegin(); iter != rend(); ++iter)
		{
			// цикл по парам (многочлен, уравнение поля)
			for (size_t i = 0; i < _n; i++)
				if (iter->LM().Test(i))
				{
					Reduce(poly.SPoly(i, *iter));
					if (poly != 0)
						return false;
				}
			// цикл по парам (многочлен, многочлен1)
			const_reverse_iterator iter1 = iter;
			for (++iter1; iter1 != rend(); ++iter1)
				if (!iter->LM().IsRelPrime(iter1->LM()))
				{
					Reduce(poly.SPoly(*iter, *iter1));
					if (poly != 0) 
						return false;
				}
			// трассировка
			Env::Trace("IsGB: %d polys", ++trace);
		}
		return true;
	}

	//! Базис факторкольца
	/*! Определяется базис факторкольца R/I как векторного пространства 
		над двоичным полем. Элементами базиса являются все мономы, которые 
		не делятся на старшие мономы базиса Гребнера идеала I.
		\remark Размерность факторкольца совпадает 
		с числом решений соответствующей I системы уравнений
		относительно существенных булевых переменных.
		\pre система должна быть базисом Гребнера, 
		размерность факторкольца должна помещаться в size_t. 
		\return размерность и базис (по ссылке polyQB). */
	template<class _O1>
	size_t QuotientBasis(MPoly<_n, _O1>& polyQB) const
	{
		// начинаем с пустого базиса
		polyQB.SetEmpty();
		if (IsEmpty()) return 0;
		// существенные переменные
		Monom<_n> vars = GatherVars();
		// если базис состоит из константы 1
		if (vars.Deg() == 0) return 0;
		// старшие мономы
		MPoly<_n, _O> mons(_order);
		GatherMinLMons(mons);
		// мономы для просмотра, начинаем с монома-константы
		MPoly<_n, _O1> tosee(polyQB.GetOrder());
		tosee = 1; 
		// пока есть что просматривать
		while (!tosee.IsEmpty())
		{
			// выбираем и удаляем из списка просмотра младший моном
			Monom<_n> mon = *(--tosee.end());
			tosee.pop_back();
			// один из старших мономов делит mon?
			typename MPoly<_n, _O1>::reverse_iterator iter;
			for ( iter = mons.rbegin(); iter != mons.rend(); iter++)
				if (*iter | mon) break;
			// элемент базиса и еще не добавлен в базис?
			if (iter == mons.rend() && !polyQB.IsContain(mon))
			{
				// добавить в базис
				polyQB.Union(mon);
				// умножаем его на всевозможные существенные переменные
				for (size_t prev = SIZE_MAX, cur = 0; cur < _n; cur++)
					if (vars.Test(cur) && !mon.Test(cur))
					{
						mon.Set(cur, 1);
						if (prev != SIZE_MAX) 
							mon.Set(prev, 0);
						prev = cur;
						tosee.Union(mon);
					}
			}
			// отладочная печать
			if ((tosee.size() % 23) == 0)
				Env::Trace("QuotientBasis: %d elems (%d tosee)", 
					polyQB.size(), tosee.size());
		}
		return polyQB.Size();
	}

	//! Размерность базиса факторкольца
	/*! Определяется размерность базиса факторкольца R/I как 
		векторного пространства над двоичным полем.
		\remark Расчеты сводятся к нахождению числа решений системы уравнений
		m = 0, m из mons, относительно неизвестных vars. 
		Здесь mons -- старшие мономы многочленов базиса Гребнера,
		а vars -- существенные переменные. Такие расчеты намного проще, 
		чем прямое нахождение базиса (см. метод QuotientBasis()).
		\pre Система должна быть базисом Гребнера. */
	ZZ<_n> QuotientBasisDim() const
	{
		ZZ<_n> dim = 0;
		// пустая система?
		if (IsEmpty()) return dim;
		// собираем минимальные старшие мономы
		MPoly<_n, _O> mons(_order);
		GatherMinLMons(mons);
		// нет решений?
		if (mons == 1) return dim;
		// собираем переменные
		Monom<_n> vars = GatherVars();
		// будем обрабатывать пары (переменные, мономы)
		typedef std::pair<Monom<_n>, MPoly<_n, _O> > Pair;
		std::list<Pair> pairs;
		// добавляем первую пару
		pairs.push_back(Pair(vars, mons));
		// пока есть пары
		while (pairs.size())
		{
			// обработаем первую пару
			typename std::list<Pair>::iterator posPair = pairs.begin();
			Pair& pair = *posPair;
			// нет уравнений?
			if (pair.second == 0)
				dim += ZZ<_n>(1).Shr(pair.first.Weight());
			// одно уравнение?
			else if (pair.second.Size() == 1)
				dim += ZZ<_n>(1).Shr(pair.first.Weight()) - ZZ<_n>(1).
					Shr(pair.first.Weight() - pair.second.LM().Weight());
			else
			{
				// ищем тривиальное уравнение x_i = 0
				// если тривиальных нет, то выбираем первое уравнение 
				typename MPoly<_n, _O>::iterator iter;
				for (iter = pair.second.end(); iter != pair.second.begin();)
					if ((--iter)->Weight() == 1) 
						break;
				// ищем переменную, которая будет исключаться
				size_t var;
				for (var = 0; var < _n; ++var)
					if (iter->Test(var)) 
						break;
				// установить var = 0
				vars = pair.first, vars.Set(var, 0);
				mons = pair.second, mons.Set(var, 0);
				// и добавить новую пару в начало списка
				pairs.push_front(Pair(vars, mons));
				// обработано нетривиальное уравнение?
				if (iter->Weight() > 1)
				{
					// установить var = 1, т.е. вернуть мономы, которые 
					// содержали var, предварительно исключив в них var
					iter = pair.second.begin();
					for (; iter != pair.second.end(); ++iter)
						if (iter->Test(var)) 
							iter->Set(var, 0), mons.Union(*iter);
					// и добавить еще одну пару в начало списка
					pairs.push_front(Pair(vars, mons));
				}
			}
			// исключить обработанную пару
			pairs.erase(posPair);
			// отладочная печать
			if ((pairs.size() % 23) == 0)
				Env::Trace("QuotientBasisDim: %d pairs remained", 
					pairs.size());
		}
		return dim;
	}

// конструкторы
public:
	//! Конструктор по умолчанию
	/*! Создается пустая система. */
	Ideal() {}
	
	//! Конструктор порядка
	/*! Создается пустая система с порядком oRight. */
	Ideal(const _O& oRight)
	{
		SetOrder(oRight);
	}

	//! Конструктор копирования
	/*! Создается копия системы iRight. */
	Ideal(const Ideal& iRight)
	{	
		SetOrder(iRight.GetOrder());
		insert(end(), iRight.begin(), iRight.end());
	}

	//! Конструктор копирования
	/*! Создается копия системы iRight с другим мономиальным порядком 
		и возможно другим числом переменных. */
	template<size_t _m, class _O1>
	Ideal(const Ideal<_m, _O1>& iRight)
	{	
		Insert(iRight);
	}
};

//! Вывод в поток
/*! Система многочленов iRight выводится в поток os. */
template<class _Char, class _Traits, size_t _n, class _O> inline 
std::basic_ostream<_Char, _Traits>& 
operator<<(std::basic_ostream<_Char, _Traits>& os, const Ideal<_n, _O>& iRight)
{
	bool waitfirst = true;
	typename Ideal<_n, _O>::const_iterator iter = iRight.begin();
	for (; iter != iRight.end(); ++iter)
	{
		os << (waitfirst ? "{\n  " : ",\n  ");
		os << *iter;
		waitfirst = false;
	}
	return os << (waitfirst ? "{}\n" : "\n}\n");
}

//! Ввод из потока
/*! Система многочленов iRight читается из потока is. 
	\par Допустимый ввод представляет собой набор из лексем-многочленов.
	Лексемы должны быть разделены знаком ",". Набор должен быть 
	выделен фигурными скобками "{" и "}". Запятая перед закрывающей 
	скобкой не допускается. Между лексемами и знаками из набора ",{}" 
	допускается наличие произвольного числа пробелов, 
	знаков табуляции и других пустых разделителей из набора " \n\r\t\v". 
	Допускается повтор лексем-многочленов.
	\par Чтение системы прекращается, если 
	1) прочитана закрывающая скобка,
	2) достигнут конец файла или встретилась неподходящая лексема.
	Во втором случае в состоянии потока будет установлен флаг 
	ios_base::failbit, без снятия которого дальнейшее чтение 
	из потока невозможно. Если ошибка чтения была связана с достижением
	конца потока, то дополнительно будет установлен флаг 
	ios_base::eofbit.
	\par По окончании ввода система iRight будет содержать 
	упрощенный (без повторов) набор многочленов. */
template<class _Char, class _Traits, size_t _n, class _O> inline
std::basic_istream<_Char, _Traits>& 
operator>>(std::basic_istream<_Char, _Traits>& is, Ideal<_n, _O>& iRight)
{	
	// предварительно обнуляем систему
	iRight.SetEmpty();
	// признак изменения
	bool changed = false;
	// захватываем поток и пропускаем пустые символы
	typename std::basic_istream<_Char, _Traits>::sentry s(is);
	// захват выполнен? 
	if (s)
	{
		// флаги ожидания символов
		bool waitforopen = true, waitforcomma = false, waitforpoly = false;
		bool afteropen = false;
		// читаем символы
		while (true)
		{
			// читаем символ из потока
			typename _Traits::int_type c1 = is.rdbuf()->sbumpc();
			// конец файла?
			if (_Traits::eq_int_type(c1, _Traits::eof()))
			{
				is.setstate(std::ios_base::eofbit | std::ios_base::failbit);
				break;
			}
			// определяем символ
			char c = is.narrow(_Traits::to_char_type(c1), '*');
			// разделитель?
			if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
				continue;
			// ожидаем {?
			if (waitforopen)
			{
				if (c != '{')
				{
					is.setstate(std::ios_base::failbit);
					break;
				}
				waitforopen = false;
				waitforpoly = true;
				afteropen = true;
				continue;
			}
			// сразу после {?
			if (afteropen)
			{
				afteropen = false;
				// обрабатываем ситуацию "{}"
				if (c == '}')
					break;
			}
			// ожидаем , или }?
			if (waitforcomma)
			{
				waitforcomma = false;
				if (c == ',')
				{
					waitforpoly = true;
					continue;
				}
				if (c != '}')
					is.setstate(std::ios_base::failbit);
				break;
			}
			// ожидаем многочлен?
			if (waitforpoly)
			{
				// возвращаем символ
				is.rdbuf()->sputbackc(_Traits::to_char_type(c1));
				// читаем многочлен
				MPoly<_n, _O> poly(iRight.GetOrder());
				is >> poly;
				// ошибка чтения?
				if (!is.good())
				{
					is.setstate(std::ios_base::failbit);
					break;
				}
				// добавить многочлен
				iRight.Insert(poly);
				waitforpoly = false;
				waitforcomma = true;
				changed = true;
				continue;
			}
			// недопустимый символ?
			if (c != '}' || changed)
				is.setstate(std::ios_base::failbit);
			break;
		}
	}
	// чтение требовалось, но не удалось?
	else if (_n > 0) 
			is.setstate(std::ios_base::failbit);
	return is;
}

} // namespace GF2

#endif // __GF2_IDEAL
