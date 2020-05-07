/*
*******************************************************************************
\file buch.h
\brief Buchberger's algorithm
\project GF2 [algebra over GF(2)]
\created 2006.01.01
\version 2020.05.07
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file buchb.h
\brief Алгоритм Бухбергера

Модуль содержит описание и реализацию класса Buchb, 
поддерживающего алгоритм Бухбергера построения базиса Гребнера.

Устойчивая реализация с условным названием BuchEx, основанная на статье 
[Агиевич С.В. Новая оптимизация алгоритма Бухбергера]
*******************************************************************************
*/

#ifndef __GF2_BUCHB
#define __GF2_BUCHB

#include "gf2/env.h"
#include "gf2/mi.h"
#include <map>

namespace GF2 {

/*!
***************************************************************************
Структура CritPair

Структура поддерживает хранение критических пар алгоритма Бухбергера.

Первым многочленом пары может быть либо неявное "уравнение поля"
x_i^2-x_i (номер i сохраняется в var1), либо явный многочлен 
(сохраняется в iter1, при этом var1 полагается равным SIZE_MAX).
Вторым многочленом пары (поле iter2) всегда является явный 
многочлен. Старшие мономы многочленов дублируются в полях lm1
и lm2.

При работе алгоритма Бухбергера второй элемент пары добавляется 
в базис Гребнера всегда позже первого.

НОК старших мономов многочленов кэшируется и используется 
при сравнении пар и при проверке делимости (см. перегруженные 
методы operator==(), operator!=(), operator|()).
Важно, что в этих операциях при var1 != SIZE_MAX НОК неявно 
домножается на переменную с номером var1.

В структуре сохраняются атрибуты многочленов пары 
(поля attr1 и attr2), а также атрибуты S-многочлена (поле attr). 
Список атрибутов поддерживается структурой Attr. В данной структуре 
должны быть обязательно определены конструктор по многочлену и 
конструктор копирования. Конструктор по многочлену вызывается для создания 
атрибутов по умолчанию для первоначальных многочленов, которые 
порождают искомый идеал. Конструктор копирования вызывается 
для переноса атрибутов в различные структуры данных на шагах алгоритма
Бухбергера. 

В порожденной структуре CP можно переопределить методы
\code
	bool operator<(const CP& cp) const 
	{...}
\endcode
Порядок < влияет на последовательность выбора пар в алгоритме Бухбергера: 
если cp1 < cp2, то cp1 обязательно будет обработана прежде cp2.
******************************************************************************
*/

template<size_t _n, class _O> struct CritPair
{
// типы
public:
	typedef typename MI<_n, _O>::iterator iterator;

// данные
public:
	//! Первый многочлен
	/*! Информация о первом многочлене пары.*/
	struct
	{
		const size_t var1; //< номер переменной, задающей уравнение поля
		const iterator iter1; //< позиция явного многочлена
		const MM<_n> lm1; //< старший моном многочлена
	};
	//! Второй многочлен
	/*! Информация о втором многочлене пары.*/
	struct
	{
		const iterator iter2; //< позиция явного многочлена
		const MM<_n> lm2; //< старший моном многочлена
	};
	//! НОК старших мономов
	/*! НОК старших мономов пары многочленов
		\remark При использовании уравнения поля (first.var != SIZE_MAX)
		НОК неявно домножается на переменную c номером first.var
		в методах сравнения и проверки делимости. */
	const MM<_n> lcm;

// S-многочлен
public:
	//! S-многочлен
	/*! Возвращается ссылка на S-многочлен пары. 
		\remark Перед вызовом данного метода должен быть обязательно 
		вызван метод Setup(), в котором S-многочлен вычисляется. */
	void GetSPoly(MP<_n, _O>& spoly) const
	{
		spoly.SetOrder(iter2->GetOrder());
		spoly = *iter2;
		if (var1 == SIZE_MAX)
			// S-многочлен для пары явных многочленов
			spoly.SPoly(*iter1);
		else
		{
			// S(x_i^2-x_i, f) = f x_i mod (x_i^2-x_i)
			spoly.SPoly(var1);
			if (spoly != 0 && spoly.LM() == iter2->LM())
				spoly += *iter2;
		}
	}

// операции
public:
	//! Равенство
	/*! Проверяется, что НОК старших мономов пары совпадает
		с НОК старших мономов пары cpRight.*/
	bool operator==(const CritPair& cpRight) const
	{
		return var1 == cpRight.var1 && lcm == cpRight.lcm;
	}

	//! Неравенство
	/*! Проверяется, что НОК старших мономов пары отличается от 
		НОК старших мономов пары cpRight.*/
	bool operator!=(const CritPair& cpRight) const
	{
		return var1 != cpRight.var1 || lcm != cpRight.lcm;
	}

	//! Делимость
	/*! Проверяется, что НОК старших мономов пары делит 
		НОК старших мономов пары cpRight.*/
	bool operator|(const CritPair& cpRight) const
	{
		return (var1 == cpRight.var1 || var1 == SIZE_MAX) && 
			(lcm | cpRight.lcm);
	}

	//! Нет зацепления?
	/*! Возвращается признак взаимной простоты старших мономов многочленов
		пары (признак отстутствия зацепления). */
	bool IsRelPrime() const
	{	
		return var1 != SIZE_MAX ? 
			lm2.Test(var1) == 0 : GCD(lm1, lm2).IsAllZero();
	}

	//! R-пара?
	/*! Возвращается признак того, что второй старший моном делит первый. */
	bool IsRPair() const
	{
		return lm2 | lm1;
	}

	//! Меньше?
	/*! Проверяется, что пара меньше cpRight.*/
	bool operator<(const CritPair& cpRight) const
	{
		return iter2->GetOrder().Compare(lcm, cpRight.lcm) < 0;
	}

	//! Печать
	/*! Отладочная печать.*/
	void Print() const
	{
		std::cout << '[';
		if (var1 != SIZE_MAX)
			Env::Print("x_%zu^2-x_%zu", var1, var1);
		else
			std::cout << lm1;
		Env::Print(", ");
		std::cout << lm2;
		Env::Print("]\n");
	}

// конструкторы
public:	
	//! Конструктор по двум итераторам и атрибутам
	/*! Создается пара многочленов (*i1, *i2). */
	CritPair(iterator i1, iterator i2) :
		var1(SIZE_MAX), iter1(i1), lm1(iter1->LM()),
		iter2(i2), lm2(i2->LM()),
		lcm(LCM(lm1, lm2))
	{
	}

	//! Конструктор по уравнению поля и итератору с атрибутом
	/*! Создается пара многочленов (x_i^2-x_i, *i2). */
	CritPair(size_t i, iterator i2) :
		var1(i), iter1(), lm1(var1),
		iter2(i2), lm2(iter2->LM()),
		lcm(LCM(lm1, lm2))
	{
	}

	//! Конструктор копирования
	/*! Создается копия пары cpRight. */
	CritPair(const CritPair& cpRight) : 
		var1(cpRight.var1), iter1(cpRight.iter1), lm1(cpRight.lm1), 
		iter2(cpRight.iter2), lm2(cpRight.lm2), 
		lcm(cpRight.lcm)
	{
	}
};

/*!
*******************************************************************************
Класс Buchb

Реализация алгоритма Бухбергера вычисления базиса Гребнера.

Модель вычислений: 
\code
	MI<n, O> gb;
	...
	Buchb<n, O, CP> bb;
	bb.Init(); // инициализация с установкой параметров по умолчанию порядка O
	bb.Init(gb); // загрузка полученного ранее базиса Гребнера
	MI<n, O> s;
	...
	bb.Update(s);
	bb.Process();
	bb.Done(s); // можно не вызывать, если найденный базис пока не нужен
	...
	MP<n, O> p;
	...
	bb.Update(p); // добавить в систему новый многочлен
	bb.Process(); // и пересчитать базис
	...
	bb.Done(s);
	...
\endcode

В реализации использованы идеи оптимизации алгоритма Бухбергера,
предложенные в работах 

[GMI87] Gebauer R. and Moller H.M. On an Installation of Buchberger’s 
        Algorithm, J. Symb. Comp., 6: 257-286, 1987.
[Agi12] Агиевич С. Усовершенствованный алгоритм Бухбергера. 
        Труды института математики НАН Беларуси, 2012, т.20, №1, с.3–13

Виртуальный метод Validate() позволяет отбраковывать многочлены 
с определенными атрибутами и не заносить их в базис Гребнера.
По умолчанию Validate() всегда возвращает true, т.е. браковка
не выполняется. Если некоторые многочлены все-таки бракуются,
то результирующая система не обязательно будет базисом Гребнера.
В конструкторе класса Buchb можно передать степень обрезки 
S-многочленов. При работе алгоритма Бухбергера S-многочлены, степень
которых больше степени обрезки игнорируются. По умолчанию степень обрезки
равняется WORD_MAX, т.е. обрезка не выполняется. При обрезке S-многочленов
результат не обязательно будет базисом Гребнера.

Метод ValidatePre() аналогичен Validate(). Отличие только в том, что
ValidatePre() выполняется до приведения S-многочлена по модулю текущей 
системы, а Validate() -- уже после приведения.
*******************************************************************************
*/

template <size_t _n, class _O, class _CP = CritPair<_n, _O>> class Buchb
{
// внутренние типы и данные
protected:
	typedef MP<_n, _O> _P;
	typedef MI<_n, _O> _I;
	typedef typename MI<_n, _O>::iterator _Iterator;
	struct _less
	{
		bool operator()(const _Iterator& left, const _Iterator& right) const
		{
			return *left < *right;
		}
	};
	typedef std::list<_CP> _CPs;
	_I _basis; // многочлены базиса Гребнера
	_I _reserve; // многочлены, исключенные r-критерием
	_CPs _pairs; // критические пары, которые надо обработать
	_CPs _pairs_processed; // обработанные критические пары
	struct
	{
		size_t pairs_processed; // обработано критических пар
		size_t reduction_to_zero; // число S-многочленов, приведенных к 0
		int max_deg; // максимальная степень S-многочленов
		size_t a_criterion; // число пар, исключенных A-критерием
		size_t b_criterion; // число пар, исключенных B-критерием
		size_t c_criterion; // число пар, исключенных C-критерием
		size_t buch_criterion; // число пар, исключенных I критерием Бухбергера
		size_t r_criterion; // число многочленов, переведенных в резерв
	} _stat; // статистика
// вычисления
protected:
	//! Внутреннее обновление
	/*! Список критических пар обновляется с учетом пар,
		которые включают многочлен системы в позиции posPoly. */
	void _Update(typename _I::iterator posPoly)
	{
		// критерий A:
		// если LM(poly) | [LM(f_i), LM(f_j)] и (f_i, f_j) не является r-парой,
		// то (f_i, f_j) можно исключить
		typename _CPs::iterator posPair = _pairs.begin();
		for (; posPair != _pairs.end();)
			if ((posPoly->LM() | posPair->lcm) && !posPair->IsRPair())
				posPair = _pairs.erase(posPair), _stat.a_criterion++;
			else 
				++posPair;

		// формируем r-пары (f_i, poly), где LM(poly) | LM(f_i)
		// удаляем многочлены f_i
		// .
		// если LM(poly) \not| LM(f_i), то LM(poly) \neq LM(f_i)
		// поэтому LM(f_i mod poly) == LM(f_i) и f_i можно заменить 
		// на f_i mod poly без изменения характеристики lcm всех S-многочленов,
		// в которые входит f_i
		_CPs newpairs;
		// цикл по многочленам базиса
		_Iterator posBasis = _basis.begin();
		for (; posBasis != _basis.end();)
		{
			if (posPoly == posBasis)
			{
				++posBasis;
				continue;
			}
			// подходящий для r-пары многочлен *posBasis?
			if (posPoly->LM() | posBasis->LM())
			{
				// переводим многочлен в резерв
				_Iterator posReserve = _reserve.Splice(_basis, posBasis++);
				// добавляем новую критическую пару
				newpairs.push_front(_CP(posReserve, posPoly));
				_stat.r_criterion++;
			}
			// упрощаем *posBasis
			else posBasis->Mod(*posPoly), ++posBasis;
		}
		// добавляем r-пары в список критических пар
		newpairs.sort();
		_pairs.merge(newpairs);

		// добавляем пары (x_i^2 - x_i, poly)  
		// с учетом первого критерия Бухбергера рассматриваем только
		// такие i, что LM(poly) содержит x_i
		for (size_t var = 0; var < _n; var++)
			// есть зацепление?
			if (posPoly->LM().Test(var))
				newpairs.push_front(_CP(var, posPoly));

		// добавляем пары (f, poly), f -- явные многочлены _basis, 
		// в список пар с учетом критериев B, С:
		// если в _basis найдется f_i т.ч. 
		// [LM(f_i), LM(poly)] | [LM(f_j), LM(poly)],
		// и LM(poly) не делит [LM(f_i), LM(f_j)] или одна из пар (f_i, f_j), 
		// (f_j,f_i) уже обработана, то 
		// a) пару (f_j, poly) можно исключить, 
		//    если [LM(f_i), LM(poly)] != [LM(f_j), LM(poly)] (критерий B);
		// б) одну из пар (f_i, poly), (f_j, poly) можно исключить,
		//    если [LM(f_i), LM(poly)] == [LM(f_j), LM(poly)] (критерий С);
		// .
		// важно: f_i не может быть уравнением поля
		// важно: при применении критерия С в первую очередь исключается пара
		//		  с зацеплением
		// важно: (f_j, poly) не может быть r-парой
		for (posBasis = _basis.begin(); posBasis != _basis.end(); ++posBasis)
		{
			if (posBasis == posPoly) continue;
			// добавляемая пара
			_CP newpair(posBasis, posPoly);
			// цикл по имеющимся парам
			posPair = newpairs.begin();
			for (; posPair != newpairs.end();)
			{
				// выполняются условия делимости?
				if ((*posPair | newpair) && 
					(!(posPoly->LM() | LCM(posPair->lm1, posBasis->LM()))))
				{
					// новая пара исключается критерием B?
					if (*posPair != newpair)
					{
						_stat.b_criterion++;
						break;
					}
					// действует критерий C
					_stat.c_criterion++;
					// новая пара исключается критерием C?
					if (posPair->IsRelPrime() || !newpair.IsRelPrime())
						break;
					// новая пара исключает пару *posPair по критерию C
					posPair = newpairs.erase(posPair);
				}
				else
					++posPair;
			}
			// пара исключена критерием B или С? к следующей
			if (posPair != newpairs.end()) 
				continue;
			// снова цикл по имеющимся парам: пробуем их исключить
			for (posPair = newpairs.begin(); posPair != newpairs.end();)
				// критерий B исключает *posPair?
				if ((newpair | *posPair) && newpair != *posPair &&
					(!(posPoly->LM() | LCM(posPair->lm1, posBasis->LM()))))
						posPair = newpairs.erase(posPair), _stat.b_criterion++;
				else
					++posPair;
			// добавляем пару
			newpairs.insert(posPair, newpair);
		}
		// применяем первый критерий Бухбергера: удаляем пары без зацепления
		for (posPair = newpairs.begin(); posPair != newpairs.end();)
			if (posPair->IsRelPrime())
				_stat.buch_criterion++,
				posPair = newpairs.erase(posPair);
			else
				++posPair;
		// слияние списков пар
		newpairs.sort();
		_pairs.merge(newpairs);
	}

protected:
	//! Предварительная проверка дополнительных условий
	/*! Выполняется проверка дополнительных условий для S-многочленов poly 
		до их приведения по модулю текущего базиса. */
	virtual bool ValidatePre(const _P& poly)
	{
		return true;
	}

	//! Проверка дополнительных условий
	/*! Выполняется проверка дополнительных условий для многочленов poly, 
		после их приведения по модулю текущего базиса. */
	virtual bool Validate(const _P& poly)
	{
		return true;
	}

public:
	//! Инициализация
	/*! Выполняется инициализация данных для работы алгоритма Бухбергера. 
		Во внутренних списках многочленов устанавливается порядок _O 
		с параметрами по умолчанию.	*/
	void Init()
	{
		// очищаем списки многочленов
		_basis.SetEmpty(); _reserve.SetEmpty();
		// устанавливаем порядок по умолчанию
		_basis.SetOrder(_O());
		_reserve.SetOrder(_O());
		// готовим списки пар
		_pairs.clear();
		_pairs_processed.clear();
		// обнуляем статистику
		std::memset(&_stat, 0, sizeof(_stat));
	}

	//! Инициализация
	/*! Выполняется инициализация данных для работы алгоритма Бухбергера. 
		В отличие от предыдущего метода загружается система многочленов 
		gb. Система gb считается базисом Гребнера и поэтому 
		критические пары для нее не строятся. 
		Условия Validate() не проверяются. 
		Во внутренних структурах данных устанавливается порядок gb, 
		даже при передаче пустой системы. */
	void Init(const _I& gb)
	{
		// очищаем списки многочленов
		_basis.SetEmpty(); _reserve.SetEmpty();
		// настраиваем порядок
		_basis.SetOrder(gb.GetOrder());
		_reserve.SetOrder(gb.GetOrder());
		// загружаем базис Гребнера
		_basis = gb;
		// очищаем списки пар
		_pairs.clear();
		_pairs_processed.clear();
		// обнуляем статистику
		std::memset(&_stat, 0, sizeof(_stat));
	}

	//! Обновление
	/*! Многочлен poly редуцируется по текущему базису. 
		Если результат упрощения не равен нулю и удовлетворяет условиям 
		Validate(), то список критических пар обновляется с учетом пар, 
		которые включают упрощенный poly. */
	template<class _O1>
	void Update(const MP<_n, _O1>& poly)
	{
		if (poly != 0 && !_basis.IsContain(poly) && ValidatePre(poly))
		{
			// добавить 
			_Iterator pos = _basis.Insert(poly);
			// упростить 
			_basis.Reduce(pos);
			// ненулевой? обновить : исключить 
			if (*pos != 0 && Validate(*pos)) 
				_basis.Move(pos), 
				_Update(pos);
			else _basis.RemoveAt(pos);
		}
	}

	//! Обновление
	/*! Многочлены системы ideal саморедуцируются. 
		Затем каждый из полученных многочленов редуцируется 
		по текущему базису. Если результат редукции не равен нулю 
		и удовлетворяет условиям Validate(), то он
		используются для создания новых критических пар. */
	template<class _O1>
	void Update(const MI<_n, _O1>& ideal)
	{
		// упрощаем систему многочленов
		_I polys(_basis.GetOrder()); 
		(polys = ideal).SelfReduce();
		// добавляем многочлены и критические пары
		for (; !polys.IsEmpty(); polys.RemoveAt(polys.begin()))
		{
			// предварительная проверка
			MP<_n, _O> poly(*polys.begin());
			if (!ValidatePre(poly))
				continue;
			// редукция
			_basis.Reduce(poly);
			if (poly == 0)
				continue;
			// удовлетворяет дополнительным условиям?
			if (Validate(poly))
			{
				_Iterator pos = _basis.Insert(poly);
				_Update(pos);
			}
			// трассировка
			Env::Trace("Buchb::Update: %zu polys left", polys.Size());
		}
	}

	//! Обработать критические пары
	/*! Обрабатываются критические пары, сформированные в методах Update(). 
		Метод Validate() может браковать некоторые S-многочлены. 
		При игнорировании ходят бы одного S-многочлена
		в результате работы не обязательно будет получен базис Гребнера. 
		Если же все S-многочлены учтены, то будет обязательно получен 
		редуцированный базис Гребнера. */
	void Process()
	{	
		_P spoly(_basis.GetOrder());
		// обрабатываем пары
		while (_pairs.size())
		{
			// найти S-многочлен 
			_pairs.begin()->GetSPoly(spoly);
			// обновить статистику
			_stat.pairs_processed++;
			// переместить обработанную пару
			_pairs_processed.splice(_pairs_processed.end(), 
				_pairs, _pairs.begin());
			// сортировать пары
			_pairs.sort();
			// проверить S-многочлен
			if (spoly == 0 || !ValidatePre(spoly))
				continue;
			// упростить S-многочлен
			_basis.Reduce(spoly);
			// нулевой?
			if (spoly == 0) 
				_stat.reduction_to_zero++;
			// удовлетворяет другим условиям?
			else if (Validate(spoly)) 
			{
				_Iterator pos = _basis.Insert(spoly);
				_Update(pos);
				_stat.max_deg = std::max(_stat.max_deg, spoly.Deg());
			}
			// трассировка
			if (_stat.pairs_processed % 23 == 0)
				Env::Trace("Buchb: %zu cp / %zu poly / %zu cp left", 
					_stat.pairs_processed, _basis.Size(), _pairs.size());
		}
	}

	//! Завершение
	/*! Окончание вычислений. По ссылке ideal 
		возвращается результат выполнения метода Process(). */
	void Done(_I& ideal) const
	{
		ideal = _basis;
		Env::Trace("");
	}

	//! Печать статистики 
	/*! Печатается статистика работы. */
	void PrintStat() const
	{
		Env::Print(
			"Buchb: %zu polynomials in the Groebner basis\n"
			"       %d/%d - min/max degree of the basis polynomials\n"
			"       %zu - critical pairs processed\n"
			"       %zu S-polynomials were reduced to 0\n"
			"       %d - max degree of S-polynomials\n"
			"       %zu/%zu/%zu times the A/B/C criteria were applied\n"
			"       %zu applications of the 1st Buchberger criterion\n"
			"       %zu polynomials were moved to the reserve\n",
			_basis.Size(), _basis.MinDeg(), _basis.MaxDeg(),
			_stat.pairs_processed, _stat.reduction_to_zero,
			_stat.max_deg,
			_stat.a_criterion, _stat.b_criterion, _stat.c_criterion,
			_stat.buch_criterion,
			_stat.r_criterion);
	}
	
	//! Конструктор
	Buchb() 
	{
		std::memset(&_stat, 0, sizeof(_stat));
	}
};

} // namespace GF2

#endif // __GF2_BUCHB