/*
*******************************************************************************
\file order.h
\brief Monom orders in GF(2)[x0,x1,...]
\project GF2 [algebra over GF(2)]
\created 2004.01.01
\version 2016.07.07
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file order.h
\brief Мономиальные порядки

Модуль содержит описание и реализацию классов мономиальных порядков
*******************************************************************************
*/

#ifndef __GF2_ORDER
#define __GF2_ORDER

#include "gf2/monom.h"
#include <functional>

namespace GF2 {

/*!
*******************************************************************************
Класс Order

-#  Базовый класс для сравнения мономов класса Monom. 
    Класс порожден от std::binary_function, что вписывает его в семейство 
	компараторов (см. std::greater, std::greater_equal, std::equal_to и др.)
-#	Порядки делятся на обычные и параметризованные в зависимости
	от необходимости использовать дополнительные данные-параметры 
	при сравнении мономов. Примером обычного порядка является lex 
	(класс OrderLex), примером параметризованного -- alex 
	(класс OrderAlex). Обычные порядки совпадают при совпадении типов,
	а параметризованные -- при совпадении как типов, так и параметров.
-#  В порожденном классе MyOrder следует определить методы 
	operator==(), Compare, operator(), operator==() и Next 
	(последний метод -- при необходимости). Прототипы методов:
	\code
	bool operator==(const MyOrder& order) const
	{
		return true if параметры *this и order совпадают
				false в противном случае
	}
	int Compare(const Monom<n>& m1, const Monom<n>& m2) const
	{
		return -1 if m1 < m2
				0 if m1 == m2
				1 if m1 > m2
	}
	bool operator()(const Monom<n>& m1, const Monom<n>& m2) const
	{
		return true if m1 > m2
			false if m1 <= m2
	}
	bool Next()(const Monom<n>& m) const
	{
		return true if m заменен на следующий моном
			false if m является последним мономом и заменен на первый.
	}
	\endcode
	Оператор присваивания (operator=) и конструктор копирования 
	в порожденных классах можно не перегружать, а использовать неявные
	функции почленного копирования и присваивания 
	(см. [Страуструп-2007, 10.4.4.1]).
-#  В порожденных классах правила сравнения формулируются для экспонент 
	a=a_0\ldots a_{n-1} и b=b_0\ldots b_{n-1} мономов. 
	Экспоненты считаются векторами с неотрицательными целочисленными 
	координатами. Степенью (deg) монома является сумма координат экспоненты.
*******************************************************************************
*/

template<size_t _n> struct Order : 
	public std::binary_function<const Monom<_n>&, const Monom<_n>&, bool>
{
	//! раскрытие числа переменных _n
	enum {n = _n};
};

/*!
*******************************************************************************
Класс OrderLex

Сравнение мономов в порядке lex: a > b, если самая правая ненулевая координата 
a - b положительна.
*******************************************************************************
*/

template<size_t _n> struct OrderLex : public Order<_n>
{
	//! Равенство порядков
	/*! Проверяется (обязательное!) совпадение с другим порядком lex. */	
	bool operator==(const OrderLex<_n>&) const
	{
		return true;
	}

	//! Сравнение lex
	/*! Мономы m1 и m2 сравниваются в порядке lex. 
		\return 1 (>), 0 (=), -1 (<).
	*/	
	int Compare(const Monom<_n>& m1, const Monom<_n>& m2) const
	{
		// воспользуемся лексикографическим сравнением слов-экспонент
		return m1.Compare(m2);
	}

	//! Проверка >
	/*! Определяется результат сравнения m1 > m2 в порядке lex. */
	bool operator()(const Monom<_n>& m1, const Monom<_n>& m2) const
	{
		return Compare(m1, m2) > 0;
	}

	//! Следующий в lex
	/*! Определяется следующий в порядке lex моном.
		Если моном является последним, то определяется первый моном.
		\return true, если построен следующий моном, и false, 
		если возвратились к первому. */	
	bool Next(Monom<_n>& m) const
	{
		// воспользуемся лексикографическим Next для слов-экспонент
		return m.Next();
	}
};

/*!
*******************************************************************************
Класс OrderGrlex

Сравнение мономов в порядке grlex: a > b, если deg(a) > deg(b), 
или deg(a) == deg(b) и a > b в lex.
*******************************************************************************
*/

template<size_t _n> struct OrderGrlex : public Order<_n>
{
	//! Равенство порядков
	/*! Проверяется (обязательное!) совпадение с другим порядком grlex. */	
	bool operator==(const OrderGrlex<_n>&) const
	{
		return true;
	}

	//! Сравнение grlex
	/*! Мономы mLeft и mRight сравниваются в порядке grlex. 
		\return 1 (>), 0 (=), -1 (<).
	*/	
	int Compare(const Monom<_n>& m1, const Monom<_n>& m2) const
	{
		int nDiff = m1.Deg() - m2.Deg();
		if (nDiff == 0)
			// воспользуемся лексикографическим сравнением слов-экспонент
			return m1.Compare(m2);
		return (nDiff < 0) ? -1 : 1;
	}

	//! Проверка >
	/*! Определяется результат сравнения m1 > m2 в порядке grlex.*/
	bool operator()(const Monom<_n>& m1, const Monom<_n>& m2) const
	{
		return Compare(m1, m2) > 0;
	}

	//! Следующий в grlex
	/*! Определяется следующий в порядке grlex моном.
		Если моном является последним, то определяется первый моном.
		\return true, если построен следующий моном, и false, если 
        возвратились к первому.
	*/	
	bool Next(Monom<_n>& m) const
	{
		size_t start = 0;
		// ищем начало серии из единиц
		while (start < _n && !m.Test(start)) start++;
		// единиц нет?
		if (start == _n)
		{
			// установить экспоненту 100..0
			if (_n > 0) m.Set(0, 1);
			return true;
		}
		// ищем окончание серии
		size_t end = start + 1;
		while (end < _n && m.Test(end)) end++;
		// серия справа?
		if (end == _n)
		{
			// все единицы?
			if (start == 0)
			{
				m.SetAllZero(); 
				return false;
			}
			// возвращаемся к первому слову с весом + 1
			m.First(end - start + 1);
			return true;
		}
		// разрываем серию
		m.Set(0, end - start - 1, 1); 
		m.Set(end - start - 1, end, 0);
		m.Set(end, 1);
		return true;
	}
};

/*!
*******************************************************************************
Класс OrderGrevlex

Сравнение мономов в порядке grevlex:
a > b, если deg(a) > deg(b) или deg(a) == deg(b) и самая левая ненулевая 
координата a - b отрицательна.
*******************************************************************************
*/

template<size_t _n> struct OrderGrevlex : public Order<_n>
{
	//! Равенство порядков
	/*! Проверяется (обязательное!) совпадение с другим порядком grevlex.*/
	bool operator==(const OrderGrevlex<_n>&) const
	{
		return true;
	}

	//! Сравнение grevlex
	/*! Мономы m1 и m2 сравниваются в порядке grevlex.
		\return 1 (>), 0 (=), -1 (<).
	*/	
	int Compare(const Monom<_n>& m1, const Monom<_n>& m2) const
	{
		int nDiff = m1.Deg() - m2.Deg();
		// степени отличаются?
		if (nDiff != 0)
			return (nDiff < 0) ? -1 : 1;

		size_t pos = 0;
		// найти самые левые различные слова
		while (pos < m1.WordSize() && m1.GetWord(pos) == m2.GetWord(pos)) 
			pos++; 
		// различий не найдено?
		if (pos == m1.WordSize()) return 0;
		// в словах экспонент левые координаты являтся младшими разрядами,
		// поэтому разряды приходится разворачивать
		word w1 = m1.GetWord(pos), w2 = m2.GetWord(pos);
		// находим самые младшие несовпадающие байты
		for (; (w1 & 255) == (w2 & 255); w1 >>= 8, w2 >>= 8);
		// делаем разворот разрядов байта
		static const u8 _revbyte[256] = 
		{
			 0,128,64,192,32,160, 96,224,16,144,80,208,48,176,112,240,
			 8,136,72,200,40,168,104,232,24,152,88,216,56,184,120,248,
			 4,132,68,196,36,164,100,228,20,148,84,212,52,180,116,244,
			12,140,76,204,44,172,108,236,28,156,92,220,60,188,124,252,
			 2,130,66,194,34,162, 98,226,18,146,82,210,50,178,114,242,
			10,138,74,202,42,170,106,234,26,154,90,218,58,186,122,250,
			 6,134,70,198,38,166,102,230,22,150,86,214,54,182,118,246,
			14,142,78,206,46,174,110,238,30,158,94,222,62,190,126,254,
			 1,129,65,193,33,161, 97,225,17,145,81,209,49,177,113,241,
			 9,137,73,201,41,169,105,233,25,153,89,217,57,185,121,249,
			 5,133,69,197,37,165,101,229,21,149,85,213,53,181,117,245,
			13,141,77,205,45,173,109,237,29,157,93,221,61,189,125,253,
			 3,131,67,195,35,163, 99,227,19,147,83,211,51,179,115,243,
			11,139,75,203,43,171,107,235,27,155,91,219,59,187,123,251,
			 7,135,71,199,39,167,103,231,23,151,87,215,55,183,119,247,
			15,143,79,207,47,175,111,239,31,159,95,223,63,191,127,255,
		};
		// и сравниваем "байты-как-числа"
		if (_revbyte[w1 & 255] < _revbyte[w2 & 255])
			return 1;
		return -1;
	}

	//! Проверка >
	/*! Определяется результат сравнения m1 > m2 в порядке grevlex. */
	bool operator()(const Monom<_n>& m1, const Monom<_n>& m2) const
	{
		return Compare(m1, m2) > 0;
	}

	//! Следующий в grevlex
	/*! Определяется следующий в порядке grevlex моном.
		Если моном является последним, то определяется первый моном.
		\return true, если построен следующий моном, и false, если 
        возвратились к первому.
	*/	
	bool Next(Monom<_n>& m) const
	{
		size_t end = _n - 1;
		// ищем окончание серии из единиц
		while (end != SIZE_MAX && !m.Test(end)) end--;
		// единиц нет (нулевая экспонента)?
		if (end == SIZE_MAX)
		{
			// установить экспоненту 100..0
			if (_n > 0) m.Set(0, 1);
			return true;
		}
		// есть 0 справа?
		if (end + 1 < _n)
		{
			// передвигаем единицу
			m.Set(end, 0); m.Set(end + 1, 1);
			return true;
		}
		// ищем начало серии из единиц
		size_t start = end - 1;
		while (start != SIZE_MAX && m.Test(start)) start--;
		// все единицы?
		if (start == SIZE_MAX)
		{
			m.SetAllZero();
			return false;
		}
		// сохраняем длину серии в end
		end -= start;
		// ищем вправо еще одну единицу
		while (--start != SIZE_MAX && !m.Test(start));
		// экспонента имеет вид 00...011...1?
		if (start == SIZE_MAX)
		{
			// переходим к следующему весу
			m.Set(0, end + 1, 1); m.Set(end + 1, _n, 0); 
			return true;
		}
		// сдвигаем серию
		m.Set(start, 0); 
		m.Set(start + 1, start + end + 2, 1); 
		m.Set(start + 2 + end, _n, 0);
		return true;
	}
};

/*!
*******************************************************************************
Класс OrderAlex

-#	Реализует сравнение мономов в порядке alex:
	векторы a и b умножаются на целочисленную матрицу A порядка n
	и полученные векторы сравниваются лексикографически, т.е.
	a > b если первая справа ненулевая координата aA - bA положительна.
-#	Известно [Robbiano, L.: On the theory of graded structures, 
	J. Symb. Comp. 2 (1986) 139–170], что любой мономиальный порядок является 
	порядком alex при некотором выборе матрицы A. Для матриц A
	должны выполняться следующие ограничения: 
	1) первый (справа) ненулевой элемент каждой строки положителен;
	2) строки линейно независимы над полем рациональных чисел.
-#	Порядок alex является примером параметризованного порядка:
	при копировании и сравнении объектов класса OrderAlex следует учитывать 
	не только типы объектов, но и встроенные в них матрицы A.
-#	Используются матрицы только с неотрицательными целыми элементами,
	которые поддерживаются типом word. Для корректной работы требуется,
	чтобы сумма элементов каждого столбца матрицы умещалась в word.
-#	Метод Next для порядка alex не поддерживается.
*******************************************************************************
*/

template<size_t _n> struct OrderAlex : public Order<_n>
{
	//! Матрица A
	word A[_n][_n];

	//! Равенство порядков
	/*! Проверяется совпадение с другим порядком alex. */	
	bool operator==(const OrderAlex<_n>& oRight) const
	{
		return ::memcmp(A, oRight._A, sizeof(A)) == 0;
	}

	//! Сравнение alex
	/*! Мономы m1 и m2 сравниваются в порядке аlex. 
		\return 1 (>), 0 (=), -1 (<).
	*/	
	int Compare(const Monom<_n>& m1, const Monom<_n>& m2) const
	{
		// цикл по столбцам справа налево
		for (size_t row = _n - 1; row != SIZE_MAX; row--)
		{
			// умножаем векторы экспонент на столбец A
			word sum1 = 0, sum2 = 0;
			for (size_t pos = 0; pos < _n; pos++)
			{
				if (m1.Test(pos)) sum1 += A[pos][row];
				if (m2.Test(pos)) sum2 += A[pos][row];
			}
			// лексикографическое сравнение результатов
			if (sum1 != sum2)
				return (sum1 < sum2) ? -1 : 1;
		}
		// при корректном выборе A в этом месте обязательно 
		// должно выполняться условие m1 == m2
		assert(m1 == m2);
		return 0;
	}

	//! Проверка >
	/*! Определяется результат сравнения m1 > m2 в порядке alex. */
	bool operator()(const Monom<_n>& m1, const Monom<_n>& m2) const
	{
		return Compare(m1, m2) > 0;
	}

	//! Конструктор по умолчанию
	/*! Конструктор по умолчанию: Устанавливается тождественная матрица _A,
		т.е. задается лексикографический порядок. */
	OrderAlex()
	{
		::memset(A, 0, sizeof(A));
		for (size_t pos = 0; pos < _n; A[pos][pos++] = 1);
	}

};

/*!
*******************************************************************************
Класс OrderRev

Сравнение мономов в обратном порядке Rev(0):
переменные переписываются в обратном порядке, а затем сравниваются в O.
*******************************************************************************
*/

template<typename _O> struct OrderRev : public Order<_O::n>
{
	//! вложенный порядок
	_O order;

	//! Равенство порядков
	/*! Проверяется совпадение с другим порядком Rev. */	
	bool operator==(const OrderRev<_O>& o) const
	{
		return order == o.order;
	}

	//! Сравнение Rev
	/*! Мономы m1 и m2 сравниваются в реверсивном порядке Rev(0).
		\return 1 (>), 0 (=), -1 (<).
	*/	
	int Compare(const Monom<_O::n>& m1, const Monom<_O::n>& m2) const
	{
		// реверсировать переменные
		Monom<_O::n> m1Rev(m1), m2Rev(m2);
		m1Rev.Reverse(), m2Rev.Reverse();
		// сравнить
		return order.Compare(m1Rev, m2Rev);
	}

	//! Проверка >
	/*! Определяется результат сравнения m1 > m2 в порядке Rev(O).*/
	bool operator()(const Monom<_O::n>& m1, const Monom<_O::n>& m2) const
	{
		return Compare(m1, m2) > 0;
	}

	//! Следующий в Rev
	/*! Определяется следующий в составном порядке Rev(O) моном.
		Если моном является последним, то определяется первый моном.
		\return true, если построен следующий моном, и false, если 
        возвратились к первому.
	*/	
	bool Next(Monom<_O::n>& m) const
	{
		m.Reverse();
		bool res = order.Next(m);
		m.Reverse();
		return res;
	}
};

/*!
*******************************************************************************
Класс OrderGr

Сравнение мономов в градуированном порядке Gr(0):
сравниваются сначала степени мономов, а при равенстве степеней выполняется
сравнение в O.

Метод Next() не поддерживается -- его нельзя реализовать эффективно, 
не уточняя порядок.
*******************************************************************************
*/

template<class _O> struct OrderGr : public Order<_O::n>
{
	//! вложенный порядок
	_O order;

	//! Равенство порядков
	/*! Проверяется совпадение с другим порядком Gr. */	
	bool operator==(const OrderGr<_O>& o) const
	{
		return order == o.order;
	}

	//! Сравнение Gr
	/*! Мономы m1 и m2 сравниваются в градуированном порядке Gr(0).
		\return 1 (>), 0 (=), -1 (<).
	*/	
	int Compare(const Monom<_O::n>& m1, const Monom<_O::n>& m2) const
	{
		// сравнить степени
		int nDiff = m1.Deg() - m2.Deg();
		// степени отличаются?
		if (nDiff != 0)
			return (nDiff < 0) ? -1 : 1;
		// сравнить в O
		return order.Compare(m1, m2);
	}

	//! Проверка >
	/*! Определяется результат сравнения m1 > m2 в порядке Gr(O).*/
	bool operator()(const Monom<_O::n>& m1, const Monom<_O::n>& m2) const
	{
		return Compare(m1, m2) > 0;
	}
};

/*!
*******************************************************************************
Класс OrderLR

Сравнение мономов в составном порядке LR(01, O2):
левые части сравниваются в O1, а при равенстве правые части сравниваются в 02.

Порядок LR(O1,O2) является исключающим: 
при нахождении базиса Гребнера исключаются левые переменные.
*******************************************************************************
*/

template<class _O1, class _O2> struct OrderLR : public Order<_O1::n + _O2::n>
{
	//! вложенный "левый" порядок
	_O1 order1;
	//! вложенный "правый" порядок
	_O2 order2;

	//! Равенство порядков
	/*! Проверяется совпадение с другим порядком LR. */	
	bool operator==(const OrderLR<_O1, _O2>& o) const
	{
		return order1 == o.order1 && order2 == o.order2;
	}

	//! Сравнение LR
	/*! Мономы m1 и m2 сравниваются в составном порядке LR(01, O2).
		\return 1 (>), 0 (=), -1 (<).
	*/	
	int Compare(const Monom<_O1::n + _O2::n>& m1, 
		const Monom<_O1::n + _O2::n>& m2) const
	{
		// найти левые части
		Monom<_O1::n> m1Left, m2Left;
		m1.GetLeft(m1Left); m2.GetLeft(m2Left);
		// сравнить левые части
		int res = order1.Compare(m1Left, m2Left);
		// совпадают?
		if (res == 0)
		{
			// найти правые части
			Monom<_O2::n> m1Right, m2Right;
			m1.GetRight(m1Right); m2.GetRight(m2Right);
			// сравнить правые части
			res = order2.Compare(m1Right, m2Right);
		}
		return res;
	}

	//! Проверка >
	/*! Определяется результат сравнения m1 > m2 в составном 
		порядке LR(01, 02).*/
	bool operator()(const Monom<_O1::n + _O2::n>& m1, 
		const Monom<_O1::n + _O2::n>& m2) const
	{
		return Compare(m1, m2) > 0;
	}

	//! Следующий в LR
	/*! Определяется следующий в составном порядке LR(O1, O2) моном.
		Если моном является последним, то определяется первый моном.
		\return true, если построен следующий моном, и false, если 
        возвратились к первому.
	*/	
	bool Next(Monom<_O1::n + _O2::n>& m) const
	{
		// Next в порядке 02
		Monom<_O2::n> mRight;
		m.GetRight(mRight);
		bool res = order2.Next(mRight);
		m.SetRight(mRight);
		// "переполнение" в 02?  
		if (!res)
		{
			// Next в порядке 01
			Monom<_O1::n> mLeft;
			m.GetLeft(mLeft);
			res = order1.Next(mLeft);
			m.SetLeft(mLeft);
		}
		return res;
	}
};

/*!
*******************************************************************************
Класс OrderRL

Сравнение мономов в составном порядке RL(01, O2): правые части сравниваются 
в O2, а при равенстве левые части сравниваются в 01.

Порядок RL(O1,O2) является исключающим: при нахождении базиса Гребнера 
исключаются правые переменные.
*******************************************************************************
*/

template<class _O1, class _O2> struct OrderRL : public Order<_O1::n + _O2::n>
{
	//! вложенный "левый" порядок
	_O1 order1;
	//! вложенный "правый" порядок
	_O2 order2;

	//! Равенство порядков
	/*! Проверяется совпадение с другим порядком RL. */	
	bool operator==(const OrderRL<_O1, _O2>& o) const
	{
		return order1 == o.order1 && order2 == o.order2;
	}

	//! Сравнение RL
	/*! Мономы m1 и m2 сравниваются в составном порядке 
		RL(01, O2).
		\return 1 (>), 0 (=), -1 (<).
	*/	
	int Compare(const Monom<_O1::n + _O2::n>& m1, 
		const Monom<_O1::n + _O2::n>& m2) const
	{
		// найти правые части
		Monom<_O2::n> m1Right, m2Right;
		m1.GetRight(m1Right); m2.GetRight(m2Right);
		// сравнить правые части
		int res = order2.Compare(m1Right, m2Right);
		// совпадают?
		if (res == 0)
		{
			// найти левые части
			Monom<_O1::n> m1Left, m2Left;
			m1.GetLeft(m1Left); m2.GetLeft(m2Left);
			// сравнить левые части
			res = order1.Compare(m1Left, m2Left);
		}
		return res;
	}

	//! Проверка >
	/*! Определяется результат сравнения m1 > m2 в составном 
		порядке RL(01, 02).*/
	bool operator()(const Monom<_O1::n + _O2::n>& m1, 
		const Monom<_O1::n + _O2::n>& m2) const
	{
		return Compare(m1, m2) > 0;
	}

	//! Следующий в RL
	/*! Определяется следующий в составном порядке RL(O1, O2) моном.
		Если моном является последним, то определяется первый моном.
		\return true, если построен следующий моном, и false, если 
        возвратились к первому.
	*/	
	bool Next(Monom<_O1::n + _O2::n>& m) const
	{
		// Next в порядке 01
		Monom<_O1::n> mLeft;
		m.GetLeft(mLeft);
		bool res = order1.Next(mLeft);
		m.SetLeft(mLeft);
		// "переполнение" в 01?
		if (!res)
		{
			// Next в порядке 02
			Monom<_O2::n> mRight;
			m.GetRight(mRight);
			res = order2.Next(mRight);
			m.SetRight(mRight);
		}
		return res;
	}
};

} // namespace GF2

#endif // __GF2_ORDER