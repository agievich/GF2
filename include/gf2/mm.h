/*
*******************************************************************************
\file mm.h
\brief Monomials in GF(2)[x0,x1,...]
\project GF2 [algebra over GF(2)]
\created 2004.01.01
\version 2020.04.22
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file mm.h
\brief Мономы от нескольких переменных

Модуль содержит описание и реализацию класса MM, поддерживающего манипуляции 
с мономами от нескольких переменных
*******************************************************************************
*/

#ifndef __GF2_MM
#define __GF2_MM

#include "gf2/ww.h"
#include "gf2/zz.h"

#include <iostream>

namespace GF2 {

/*!
*******************************************************************************
Класс MM

Поддерживает операции с мономами многочленов от переменных x0, x1,... x_{n-1}.
Коэффициент монома всегда равняется 1. 

Количество переменных указывается при создании монома:
\code
    MM<n> m;
\endcode
Мономы приводятся по модулю идеала (x0^2-x0, x1^2-x1,...,x_{n-1}^2-x_{n-1}).
Это значит, что степени вхождения в моном переменных не превосходят 1. 

Моном кодируется словом-экспонентой b0 b1 ... b_{n-1}: b_i равняется 1, 
если и только если моном содержит переменную x_i.
Манипуляции над экспонентой поддерживаются базовым классом WW<n>.

Нулевая строка кодирует моном-константу 1. Моном-константа 0 не поддерживается.
Операции сравнения, присваивания и т.д. булевых констант не реализованы 
(нет большой необходимости, проблема не поддерживаемого нуля и др. причины).

Удобно записывать моном в виде произведения \prod x_i^{b_i},
cледуя правилам: x^0 = 1, x^1 = x.

Метод Calc() возвращает значение \prod a_i^{b_i}, где бинарное слово 
a0 a1 ... a_{n-1} передается в виде вектора.
*******************************************************************************
*/

template<size_t _n> class MM : public WW<_n>
{
protected:
	using WW<_n>::_wcount;
	using WW<_n>::_words;
public:
	using WW<_n>::Set;
	using WW<_n>::Weight;
// базовые операции
public:
	//! Вычисление значения
	/*! Вычисляется значение монома при подстановке на места переменных 
        символов слова val. */
	bool Calc(const WW<_n>& val) const
	{	
		for (size_t pos = 0; pos < _wcount; ++pos)
			if ((val.GetWord(pos) | ~_words[pos]) != WORD_MAX)
				return false;
		return true;
	}

	//! Степень
	/*! Определяется степень монома (вес экспоненты). 
		\remark Степень нулевого многочлена равняется -1,
		поэтому для дальнейшей совместимости вес экспоненты возвращается 
		как знаковое целое. */
	int Deg() const
	{	
		return int(Weight());
	}

// операции
public:
	//! Вычисление значения
	/*! Вычисляется значение монома при подстановке на места переменных 
        символов слова val. */
	bool operator()(const WW<_n>& val) const
	{	
		return Calc(val);
	}

	//! Плюс
	/*! Унарный плюс (пустой оператор). */
	MM& operator+()
	{	
		return *this;
	}

	//! Произведение
	/*! Моном домножается на mRight (логическое OR экспонент).*/
	template<size_t _m>
	MM& operator*=(const MM<_m>& mRight)
	{	
		WW<_n>::operator|=(mRight);
		return *this;
	}

	//! НОК
	/*! Моному присваивается НОК мономов m1 и m2.*/
	MM& LCM(const MM& m1, const MM& m2)
	{
		for (size_t pos = 0; pos < _wcount; ++pos)
			_words[pos] = m1.GetWord(pos) | m2.GetWord(pos);
		return *this;
	}

	//! НОД
	/*! Моному присваивается НОД мономов m1 и m2.*/
	MM& GCD(const MM& m1, const MM& m2)
	{
		for (size_t pos = 0; pos < _wcount; ++pos)
			_words[pos] = m1.GetWord(pos) & m2.GetWord(pos);
		return *this;
	}

	//! Взаимная простота
	/*! Проверяется взаимная простота с мономом mRight.*/
	bool IsRelPrime(const MM& mRight) const
	{
		for (size_t pos = 0; pos < _wcount; pos++)
			if (_words[pos] & mRight.GetWord(pos))
				return false;
		return true;
	}

	//! Взаимная простота
	/*! Проверяется взаимная простота с мономом mRight 
		с другим числом переменных.*/
	template<size_t _m>
	bool IsRelPrime(const MM<_m>& mRight) const
	{
		size_t pos;
		for (pos = 0; pos < min(_wcount, mRight.WordSize()); ++pos)
			if (_words[pos] & mRight.GetWord(pos)) return false;
		for (; pos < mRight.WordSize(); pos++)
			if (mRight.GetWord(pos)) return false;
		for (; pos < _wcount; pos++)
			if (_words[pos]) return false;
		return true;
	}

	//! Проверка делимости на
	/*! Проверка делимости на моном mRight. */
	bool IsDivisibleBy(const MM& mRight) const
	{	
		for (size_t pos = 0; pos < _wcount; ++pos)
			if (mRight.GetWord(pos) & ~_words[pos])
				return false;
		return true;
	}

	//! Проверка делимости на
	/*! Проверка делимости на моном mRight с другим числом переменных. */
	template<size_t _m>
	bool IsDivisibleBy(const MM<_m>& mRight) const
	{	
		size_t pos;
		for (pos = 0; pos < std::min(_wcount, mRight.WordSize()); ++pos)
			if (mRight.GetWord(pos) & ~_words[pos])
				return false;
		for (; pos < mRight.WordSize(); pos++)
			if (mRight.GetWord(pos))
				return false;
		return true;
	}

	//! Признак делимости
	/*! Проверка того, что моном делит моном mRight. */
	bool IsDivide(const MM& mRight) const
	{	
		for (size_t pos = 0; pos < _wcount; ++pos)
			if (_words[pos] & ~mRight.GetWord(pos))
				return false;
		return true;
	}

	//! Признак делимости
	/*! Проверка того, что моном делит моном mRight 
		с другим числом переменных. */
	template<size_t _m>
	bool IsDivide(const MM<_m>& mRight) const
	{	
		size_t pos;
		for (pos = 0; pos < min(_wcount, mRight.WordSize()); ++pos)
			if (_words[pos] & ~mRight.GetWord(pos))
				return false;
		for (; pos < _wcount; ++pos)
			if (_words[pos])
				return false;
		return true;
	}

	//! Проверка деления
	/*! Проверка того, что моном делит моном mRight возможно
		с другим числом переменных. 
		\remark Запись a | b соответствует устоявшимся 
		математическим обозначениям. */
	template<size_t _m>
	bool operator|(const MM<_m>& mRight) const
	{	
		return IsDivide(mRight);
	}

	//! Деление
	/*! Деление монома на моном mRight. 
		\pre mRight | *this. */
	MM& operator/=(const MM& mRight)
	{	
		assert(IsDivisibleBy(mRight));
		for (size_t pos = 0; pos < _wcount; ++pos)
			_words[pos] &= ~mRight.GetWord(pos);
		return *this;
	}

	//! Деление
	/*! Деление монома на моном mRight с другим числом переменных. 
		\pre mRight | *this. */
	template<size_t _m>
	MM& operator/=(const MM<_m>& mRight)
	{	
		assert(IsDivisibleBy(mRight));
		for (size_t pos = 0; pos < std::min(_wcount, mRight.WordSize()); ++pos)
			_words[pos] &= ~mRight.GetWord(pos);
		return *this;
	}

// конструкторы
public:
	//! Конструктор по умолчанию
	/*! Создается моном-константа 1. */
	MM() {}
	
	//! Конструктор копирования
	/*! Создается копия монома mRight. */
	MM(const MM& mRight) : WW<_n>(mRight) {}

	//! Конструктор копирования
	/*! Создается копия монома mRight c другим числом переменных. */
	template<size_t _m> 
	MM(const MM<_m>& mRight) : WW<_n>(mRight) {}
	
	//! Конструктор линейных мономов
	/*! Создается моном x_i. */
	explicit MM(size_t i)
	{	
		Set(i, 1);
	}

	//! Конструктор квадратичных мономов
	/*! Создается моном x_i x_j. */
	explicit MM(size_t i, size_t j)
	{	
		Set(i, 1), Set(j, 1);
	}

	//! Конструктор кубических мономов
	/*! Создается моном x_i x_j x_k. */
	explicit MM(size_t i, size_t j, size_t k)
	{	
		Set(i, 1), Set(j, 1), Set(k, 1); 
	}

	//! Конструктор по списку
	/*! Создается моном x_i x_j x_k x_l x_m. */
	MM(const std::initializer_list<size_t> l)
	{
		for (auto iter = l.begin(); iter != l.end(); ++iter)
			Set(*iter, 1);
	}
};

//! НОД мономов
/*! Определяется наибольший общй делитель мономов mLeft и mRight. */
template<size_t _n, size_t _m> decltype(auto)
GCD(const MM<_n>& mLeft, const MM<_m>& mRight)
{
	MM<std::max(_n, _m)> m(mLeft);
	m &= mRight;
	return m;
}

//! Умножение мономов
/*! Определяется произведение мономов mLeft и mRight. */
template<size_t _n, size_t _m> inline decltype(auto)
operator*(const MM<_n>& mLeft, const MM<_m>& mRight)
{
	MM<std::max(_n, _m)> m(mLeft);
	m *= mRight;
	return m;
}

//! НОК мономов
/*! Определяется наименьшее общее кратное мономов mLeft и mRight. */
template<size_t _n, size_t _m> inline decltype(auto)
LCM(const MM<_n>& mLeft, const MM<_m>& mRight)
{
	MM<std::max(_n, _m)> m(mLeft);
	m |= mRight;
	return m;
}

//! Деление мономов
/*! Определяется результат деления монома mLeft на mRight. 
	\pre mLeft.IsDivisibleBy(mRight). */
template<size_t _n, size_t _m> inline decltype(auto)
operator/(const MM<_n>& mLeft, const MM<_m>& mRight)
{
	MM<std::max(_n, _m)> m(mLeft);
	m /= mRight;
	return m;
}

//! Вывод в поток
/*! Моном mRight выводится в поток os. */
template<class _Char, class _Traits, size_t _n> inline 
std::basic_ostream<_Char, _Traits>& 
operator<<(std::basic_ostream<_Char, _Traits>& os, const MM<_n>& mRight)
{
	bool waitfirst = true;
	for (size_t pos = 0; pos < mRight.Size(); ++pos)
		if (mRight.Test(pos))
		{
			if (!waitfirst) 
				os << " ";
			os << 'x' << pos;
			waitfirst = false;
		}
	if (waitfirst) 
		os << "1";
	return os;
}

//! Ввод из потока
/*! Моном mRight читается из потока is. 
	\remark Допустимый ввод представляет собой набор лексем "1" и "xi", 
	разделенных пробелами, знаками табуляции и другими пустыми разделителями
	из набора " \n\r\t\v".
	Набор лексем должен удовлетворять следующим ограничениям:
	-	может быть только единственная лексема "1";
	-	индексы i лексем "xi" должны быть различны и должны лежать в
		интервале [0, n).
	Индексы могут быть дополнены слева незначащими нулями: 
	допустим ввод как x01, так и x1.
	\remark Формат ввода: "1" или "x_i1 пробелы x_i2 ... x_ik", где
	индексы i1, i2, ..., ik различны и лежат в интервале [0, n).
	\par Чтение монома прекращается, если 
	1) прочитана лексема "1",
	2) достигнут конец файла или встретилась неподходящая лексема,
	3) при разборе лексем возникла ошибка. 
	Если не прочитано ни одной лексемы, либо возникла ошибка при разборе
	лексем, то в состоянии потока будет установлен флаг 
	ios_base::failbit, без снятия которого дальнейшее чтение из потока 
	невозможно. */
template<class _Char, class _Traits, size_t _n> inline
std::basic_istream<_Char, _Traits>& 
operator>>(std::basic_istream<_Char, _Traits>& is, MM<_n>& mRight)
{	
	// предварительно обнуляем моном
	mRight.SetAll(0);
	// признак изменения
	bool changed = false;
	// захватываем поток и пропускаем пустые символы
	typename std::basic_istream<_Char, _Traits>::sentry s(is);
	// захват выполнен? 
	if (s)
	{
		// флаг ожидания разделителя
		bool waitfordelimiter = false;
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
			// константа? первая?
			if (c == '1' && !changed)
				break;
			// префикс переменной?
			if (c == 'x')
			{
				// ожидали разделитель?
				if (waitfordelimiter)
				{
					is.setstate(std::ios_base::failbit);
					break;
				}
				// читаем индекс переменной
				std::ios_base::fmtflags flags = is.flags(std::ios_base::dec);
				ZZ<_n> index;
				(is >> index).flags(flags);
				// ошибка чтения? неверный индекс? переменная уже прочитана?
				if (!is.good() || index >= ZZ<_n>(_n) || mRight[index] != 0)
				{
					is.setstate(std::ios_base::failbit);
					break;
				}
				// установить новую переменную
				mRight[index] = 1;
				changed = true;
				// ожидаем разделитель
				waitfordelimiter = true;
				continue;
			}
			// разделитель?
			if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
				waitfordelimiter = false;
			// прочитанный символ нельзя обработать, возвращаем его и выходим
			else
			{
				is.rdbuf()->sputbackc(_Traits::to_char_type(c1));
				if (!changed)
					is.setstate(std::ios_base::failbit);
				break;
			}
		}
	}
	// чтение требовалось, но не удалось?
	else
		is.setstate(std::ios_base::failbit);
	return is;
}

} // namespace GF2

#endif // __GF2_MM