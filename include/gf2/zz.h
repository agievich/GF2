/*
*******************************************************************************
\file zz.h
\brief Binary words as integers
\project GF2 [algebra over GF(2)]
\created 2004.01.01
\version 2020.07.14
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file zz.h
\brief Двоичные слова как числа

Модуль содержит описание и реализацию класса ZZ, поддерживающего манипуляции 
с элементами кольца вычетов ZZ / 2^n ZZ.
*******************************************************************************
*/

#ifndef __GF2_ZZ
#define __GF2_ZZ

#include "gf2/defs.h"
#include "gf2/ww.h"
#include <iomanip>
#include <cstring>
#include <vector>

namespace GF2 {

/*!
*******************************************************************************
Класс ZZ

Поддерживает операции с элементами кольца ZZ/2^n ZZ ={0,1,\ldots,2^n-1}, n > 0.

Арифметические операции (сложение, умножение, вычитание) выполняются также как
для обычных целых с обязательным приведением результата по модулю 2^n.

Некоторые методы базового класса WW, которые возвращают ссылку на WW,
переопределяются так, чтобы возвращать ссылку на ZZ.

В следующем фрагменте
\code
	ZZ<2> w1(3);
	ZZ<3> w2(7);
	ZZ<5> w3(w1 * w2);
\endcode
переменная w3 примет значение 5, а не 21, как можно было расчитывать.
Дело в том, что результатом выполнения w1 * w2 является экземпляр 
класса ZZ<3>, который затем расширяется до экземпляра ZZ<5>.

\todo Посмотреть внимательно умножение чисел разных длин в ZZ 
(не обрабатывается последний carry).
*******************************************************************************
*/

template<size_t _n> class ZZ : public WW<_n>
{
protected:
	using WW<_n>::_wcount;
	using WW<_n>::_words;
public:
	using WW<_n>::Next;
	using WW<_n>::SetAllZero;
	using WW<_n>::SetWord;
	using WW<_n>::Test;
	using WW<_n>::Trim;
	using WW<_n>::Prev;
// базовые операции
public:
	//! Сдвиг в сторону младших разрядов
	/*! Сдвиг символов слова в сторону младших разрядов
		(деление на степень 2). */
	ZZ& ShLo(size_t shift)
	{	
		WW<_n>::ShLo(shift);
		return *this;
	}

	//! Сдвиг в сторону старших разрядов
	/*! Сдвиг символов слова в сторону старших разрядов
		(умножение на степень 2). */
	ZZ& ShHi(size_t shift)
	{	
		WW<_n>::ShHi(shift);
		return *this;
	}

	//! Циклический сдвиг в сторону младших разрядов
	/*! Циклический сдвиг символов слова в сторону младших разрядов. */
	ZZ& RotLo(size_t shift)
	{	
		WW<_n>::RotLo(shift);
		return *this;
	}

	//! Циклический сдвиг в сторону старших разрядов
	/*! Циклический сдвиг символов слова в сторону старших разрядов. */
	ZZ& RotHi(size_t shift)
	{	
		WW<_n>::RotHi(shift);
		return *this;
	}

	//! Четное?
	/*! Возвращается признак четности числа. */
	bool IsEven() const
	{	
		return (_words[0] & 1) == 0;
	}

	//! Нечетное?
	/*! Возвращается признак нечетности числа. */
	bool IsOdd() const
	{	
		return (_words[0] & 1) != 0;
	}

	//! Логарифм
	/*! Возвращается минимальное k такое, что число меньше 2^k. */
	size_t Log() const
	{
		size_t k = _n;
		while (k)
            if (Test(k - 1)) 
				return k;
			else 
				--k;
		return k;		
	}

// арифметика
public:
	//! Префиксный инкремент
	/*! Число увеличивается на 1. Возвращается результат. */
	ZZ& operator++()
	{	
		Next();
		return *this;
	}

	//! Постфиксный инкремент
	/*! Число увеличивается на 1. Вовзращается первоначальное значение. */
	ZZ operator++(int)
	{	
		ZZ save(*this);
		Next();
		return save;
	}

	//! Префиксный декремент
	/*! Число уменьшается на 1. Возвращается результат. */
	ZZ& operator--()
	{	
		Prev();
		return *this;
	}

	//! Постфиксный декремент
	/*! Число уменьшается на 1. Возращается первоначальное значение. */
	ZZ operator--(int)
	{	
		ZZ save(*this);
		Prev();
		return save;
	}

	//! Плюс
	/*! Унарный плюс (пустой оператор). */
	ZZ& operator+()
	{	
		return *this;
	}

	//! Минус
	/*! Унарный минус (аддитивно обратный по модулю). */
	ZZ operator-() const
	{	
		ZZ res(*this);
		res.FlipAll().Next();
		return res;
	}

	//! Сложение
	/*! К числу добавляется машинное слово wRight. */
	ZZ& operator+=(word wRight)
	{	
		if ((_words[0] += wRight) < wRight)
		{
			size_t pos = 1;
			for (; pos < _wcount && ++_words[pos] == 0; ++pos);
		}
		Trim();
		return *this;
	}

	//! Сложение
	/*! К числу добавляется число zRight. */
	ZZ& operator+=(const ZZ& zRight)
	{	
		word carry = 0;
		for (size_t pos = 0; pos < _wcount; ++pos)
			if ((_words[pos] += carry) < carry) 
				_words[pos] = zRight.GetWord(pos);
            else 
				carry = (_words[pos] += zRight.GetWord(pos)) < 
					zRight.GetWord(pos);
		Trim();
		return *this;
	}

	//! Сложение
	/*! К числу добавляется число zRight с другим числом разрядов. */
	template<size_t _m>
	ZZ& operator+=(const ZZ<_m>& zRight)
	{	
		word carry = 0;
		size_t pos;
		for (pos = 0; pos < std::min(_wcount, zRight.WordSize()); ++pos)
			if ((_words[pos] += carry) < carry) 
				_words[pos] = zRight.GetWord(pos);
            else 
				carry = (_words[pos] += zRight.GetWord(pos)) < 
					zRight.GetWord(pos);
		if (carry)
			for (; pos < _wcount && ++_words[pos] == 0; ++pos);
		Trim();
		return *this;
	}

	//! Вычитание
	/*! Из числа вычитается машинное слово wRight. */
	ZZ& operator-=(word wRight)
	{	
 		if ((_words[0] -= wRight) > WORD_MAX - wRight)
		{
			size_t pos = 1;
			for (; pos < _wcount && --_words[pos] == WORD_MAX; ++pos);
		}
		Trim();
		return *this;
	}

	//! Вычитание
	/*! Из числа вычитается число zRight. */
	ZZ& operator-=(const ZZ& zRight)
	{	
		word borrow = 0;
		for (size_t pos = 0; pos < _wcount; pos++)
			if ((_words[pos] -= borrow) > WORD_MAX - borrow) 
				_words[pos] -= zRight.GetWord(pos);
            else 
				borrow = (_words[pos] -= zRight.GetWord(pos)) > 
					WORD_MAX - zRight.GetWord(pos);
		Trim();
		return *this;
	}

	//! Вычитание
	/*! Из числа вычитается число zRight с другим число разрядов. */
	template<size_t _m>
	ZZ& operator-=(const ZZ<_m>& zRight)
	{	
		word borrow = 0;
		size_t pos = 0;
		for (; pos < std::min(_wcount, zRight.WordSize()); ++pos)
			if ((_words[pos] -= borrow) > WORD_MAX - borrow) 
				_words[pos] -= zRight.GetWord(pos);
            else 
				borrow = (_words[pos] -= zRight.GetWord(pos)) > 
					WORD_MAX - zRight.GetWord(pos);
		if (borrow)
			for (; pos < _wcount && --_words[pos] == WORD_MAX; ++pos);
		Trim();
		return *this;
	}

	//! Умножение
	/*! Число умножается на машинное слово wRight. */
	ZZ& operator*=(word wRight)
	{	
		// сохранить и обнулить
		ZZ save(*this);
		*this = 0;
		if (wRight != 0)
		{
			// цикл по словам
			word carry = 0;
			for (size_t pos = 0; pos < _wcount; ++pos)
			{
				dword mul(wRight);
				mul *= save.GetWord(pos);
				mul += carry;
				mul += _words[pos];
				_words[pos] = word(mul);
				carry = word(mul >> B_PER_W);
			}
			Trim();
		}
		return *this;
	}

	//! Умножение
	/*! Число умножается на число zRight с возможно 
		другим числом разрядов. */
	template<size_t _m>
	ZZ& operator*=(const ZZ<_m>& zRight)
	{	
		ZZ res;
		// цикл по словам zRight
		for (size_t posRight = 0; posRight < zRight.WordSize(); ++posRight)
		{
			// цикл по словам this
			word carry = 0;
			for (size_t pos = 0; pos + posRight < _wcount; ++pos)
			{
				dword mul(zRight.GetWord(posRight));
				mul *= _words[pos];
				mul += carry;
				mul += res.GetWord(pos + posRight);
				res.SetWord(pos + posRight, word(mul));
				carry = word(mul >> B_PER_W);
			}
		}
		res.Trim();
		return operator=(res);
	}

	//! Мультипликативно обратный
	/*! Определяется мультиплмкативно обратный элемент 
		по модулю 2^n. 
		\pre число должно быть нечетным. */
		ZZ& Inv()
		{	
			assert(IsOdd());
			ZZ mul(*this), inv(1);
			for (size_t t = 1; t < _n; ++t)
			{
				ShHi(1);
				if (mul[t])
					inv[t] = 1, mul += *this;
			}
			return operator=(inv);
		} 

	//! Деление 
	/*! Выполняется деление на ненулевое машинное слово wRight.
		Частное сохраняется в самом числе, а остаток возвращается 
		в wRight. */
	ZZ& Div(word& wRight)
	{	
		assert(wRight != 0);
		word rem = 0;
		if (_wcount == 1)
		{
			rem = _words[0] % wRight;
			_words[0] /= wRight;
			wRight = rem;
			return *this;
		}
		dword divisor;
		for (size_t pos = _wcount - 1; pos != SIZE_MAX; --pos)
		{
			// делим (предыдущий_остаток, текущий_разряд) на wRight
			divisor = rem;
			divisor <<= B_PER_W;
			divisor |= _words[pos];
			_words[pos] = word(divisor / wRight);
			rem = word(divisor % wRight);
		}
		wRight = rem;
		return *this;
	}

	//! Частное
	/*! Определяется частное от деления на ненулевое машинное 
		слово wRight. */
	ZZ& operator/=(word wRight)
	{	
		return Div(wRight);
	}

	//! Остаток
	/*! Определяется остаток от деления на ненулевое машинное 
		слово wRight. */
	ZZ& operator%=(word wRight)
	{	
		Div(wRight);
		return operator=(wRight);
	}

	//! Деление 
	/*! Выполняется деление на ненулевое число zRight.
		Частное сохраняется в самом числе, а остаток возвращается 
		в zRight. */
	template<size_t _m>
	ZZ& Div(ZZ<_m>& zRight)
	{	
		assert(!zRight.IsAllZero());
		// делимое меньше делителя?
		if (*this < zRight)
		{
			zRight = *this;
			SetAllZero();
			return *this;
		}
		// делим на одноразрядное число?
		if (zRight == zRight.GetWord(0))
		{
			word rem = zRight.GetWord(0);
			Div(rem);
			zRight = rem;
			return *this;
		}
		// число значащих разрядов делителя
		size_t digits = zRight.WordSize() - 1;
		for (; zRight.GetWord(digits) == 0; --digits);
		// определить сдвиг нормализации
		word shift = 0;
		for (;(zRight.GetWord(digits) << shift) < WORD_HI; ++shift);
		// делимое, в которое поместится результат нормализации
		ZZ<_n + B_PER_W - 1> divident(*this);
		// делитель, длина которого кратна длине машинного слова
		ZZ<(_m + B_PER_W - 1) / B_PER_W * B_PER_W> divisor(zRight);
		// выполнить нормализацию
		divident.ShHi(shift);
		divisor.ShHi(shift);
		// сохранить старшие разряды делителя
		ZZ<3 * B_PER_W> divisorHi;
		divisorHi.SetWord(0, divisor.GetWord(digits - 1));
		divisorHi.SetWord(1, divisor.GetWord(digits));
		// цикл по разрядам делимого
		// сохраняем частное в *this, а остаток -- в divident
		for (size_t pos = divident.WordSize(); pos > digits; --pos)
		{
			// вычисление пробного частного
			dword q = (pos == divident.WordSize()) ? 0 : divident.GetWord(pos);
			q <<= B_PER_W;
			q |= divident.GetWord(pos - 1);
			q /= divisor.GetWord(digits);
			if (q > WORD_MAX) 
				q = WORD_MAX;
			// определить старшие разряды делимого
			ZZ<3 * B_PER_W> dividentHi;
			dividentHi.SetWord(0, divident.GetWord(pos - 2));
			dividentHi.SetWord(1, divident.GetWord(pos - 1));
			dividentHi.SetWord(2, pos == divident.WordSize() ? 
				0 : divident.GetWord(pos));
			// уточнить пробное частное
			for (; divisorHi * word(q) > dividentHi; --q);
			// умножить делитель на пробное_частное и степень основания
			ZZ<_n + B_PER_W - 1> mul(divisor);
			mul.ShHi(B_PER_W * (pos - digits - 1));
			mul *= word(q);
			if (divident < mul)
			{
				// корректировка пробного частного
				--q;
				// и результата умножения
				mul -= ZZ<_n + B_PER_W - 1>(divisor).
					ShHi(B_PER_W * (pos - digits - 1));
			}
			// вычесть
			assert(divident >= mul);
			divident -= mul;
			// установить разряд частного
			if (pos - digits <= _wcount)
				SetWord(pos - digits - 1, word(q));
		}
		// денормализация
		divident.ShLo(shift);
		// переписываем остаток
		zRight = divident;
		return *this;
	}

	//! Частное
	/*! Определяется частное от деления на ненулевое число zRight. */
	template<size_t _m> 
	ZZ& operator/=(const ZZ<_m>& zRight)
	{	
		return Div(ZZ<_m>(zRight));
	}

	//! Остаток
	/*! Определяется остаток от деления на ненулевое число zRight. */
	template<size_t _m> 
	ZZ& operator%=(const ZZ<_m>& zRight)
	{	
		ZZ<_m> mod(zRight);
		Div(mod);
		return operator=(mod);
	}

// операции
public:
	//! Присваивание
	/*! Присваивание числу значения-числа zRight. */
	template<size_t _m>
	ZZ& operator=(const ZZ<_m>& zRight)
	{	
		WW<_n>::operator=(zRight);
		return *this;
	}

	//! Присваивание
	/*! Присваивание числу значения-машинного слова wRight. */
	ZZ& operator=(word wRight)
	{	
		WW<_n>::operator=(wRight);
		return *this;
	}

// конструкторы
public:
	//! Конструктор по умолчанию
	/*! Создается нулевое слово. */
	ZZ() {}
	
	//! Конструктор по машинному слову
	/*! Создается число со значением wRight mod 2^n. */
	ZZ(word wRight) : WW<_n>(wRight) {}

	//! Конструктор копирования
	/*! Создается копия числа zRight. */
	ZZ(const ZZ& zRight) : WW<_n>(zRight) {}

	//! Конструктор копирования
	/*! Создается копия числа zRight другой размерности. */
	template<size_t _m> 
	ZZ(const ZZ<_m>& zRight) : WW<_n>(zRight) {}
};

//! Сложение
/*! Определяется сумма чисел zLeft и zRight. */
template<size_t _n, size_t _m> inline auto
operator+(const ZZ<_n>& zLeft, const ZZ<_m>& zRight)
{
	ZZ<std::max(_n, _m)> z(zLeft);
	z += zRight;
	return z;
}

//! Сложение
/*! Определяется сумма машинного слова wLeft и числа zRight. */
template<size_t _n> inline auto
operator+(word wLeft, const ZZ<_n>& zRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(zRight);
	z += wLeft;
	return z;
}

//! Сложение
/*! Определяется сумма числа zLeft и машинного слова wRight. */
template<size_t _n> inline auto
operator+(const ZZ<_n>& zLeft, word wRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(zLeft);
	z += wRight;
	return z;
}

//! Вычитание
/*! Определяется разность чисел zLeft и zRight. */
template<size_t _n, size_t _m> inline auto
operator-(const ZZ<_n>& zLeft, const ZZ<_m>& zRight)
{
	ZZ<std::max(_n, _m)> z(zLeft);
	z -= zRight;
	return z;
}

//! Вычитание
/*! Определяется разность машинного слова wLeft и числа zRight. */
template<size_t _n> inline auto
operator-(word wLeft, const ZZ<_n>& zRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(zRight);
	z -= wLeft;
	return z;
}

//! Вычитание
/*! Определяется разность числа zLeft и машинного слова wRight. */
template<size_t _n> inline auto
operator-(const ZZ<_n>& zLeft, word wRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(zLeft);
	z -= wRight;
	return z;
}

//! Умножение
/*! Определяется произведение чисел zLeft и zRight. */
template<size_t _n, size_t _m> inline auto
operator*(const ZZ<_n>& zLeft, const ZZ<_m>& zRight)
{
	ZZ<std::max(_n, _m)> z(zLeft);
	z *= zRight;
	return z;
}

//! Умножение
/*! Определяется произведение машинного слова wLeft и числа zRight.*/
template<size_t _n> inline auto
operator*(word wLeft, const ZZ<_n>& zRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(zRight);
	z *= wLeft;
	return z;
}

//! Умножение
/*! Определяется произведение числа zLeft и машинного слова wRight. */
template<size_t _n> inline auto
operator*(const ZZ<_n>& zLeft, word wRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(zLeft);
	z *= wRight;
	return z;
}

//! Частное
/*! Определяется частное от деления числа zLeft на zRight. */
template<size_t _n, size_t _m> inline auto
operator/(const ZZ<_n>& zLeft, const ZZ<_m>& zRight)
{
	ZZ<std::max(_n, _m)> z(zLeft);
	z /= zRight;
	return z;
}

//! Частное
/*! Определяется частное от деления числа zLeft 
на машинное слово wRight.*/
template<size_t _n> inline auto
operator/(const ZZ<_n>& zLeft, word wRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(zLeft);
	z /= wRight;
	return z;
}

//! Частное
/*! Определяется частное от деления машинного слова wLeft 
на число zRight.*/
template<size_t _n> inline auto
operator/(word wLeft, const ZZ<_n>& zRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(wLeft);
	z /= zRight;
	return z;
}

//! Остаток
/*! Определяется остаток от деления числа zLeft на zRight. */
template<size_t _n, size_t _m> inline auto
operator%(const ZZ<_n>& zLeft, const ZZ<_m>& zRight)
{
	ZZ<std::max(_n, _m)> z(zLeft);
	z %= zRight;
	return z;
}

//! Остаток
/*! Определяется остаток от деления числа zLeft 
на машинное слово wRight.*/
template<size_t _n> inline auto
operator%(const ZZ<_n>& zLeft, word wRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(zLeft);
	z %= wRight;
	return z;
}

//! Остаток
/*! Определяется остаток от деления машинного слова wLeft 
на число zRight.*/
template<size_t _n> inline auto
operator%(word wLeft, const ZZ<_n>& zRight)
{
	ZZ<std::max(_n, sizeof(word) * 8)> z(wLeft);
	z %= zRight;
	return z;
}

//! Вывод в поток
/*! Вывод числа zRight в поток os.
	Справа -- младшие символы, слева -- младшие. 
	Нулевые старшие символы не выводятся.
	Число выводится в шестнадцатеричном виде при установке 
	флага форматирования ios_base::hex
	и в десятичном виде для всех остальных флагов.
	Если установлен флаг ios_base::showbase,
	то при шестнадцатеричном выводе дописывается префикс 0x. */
template<class _Char, class _Traits, size_t _n> inline 
std::basic_ostream<_Char, _Traits>& 
operator<<(std::basic_ostream<_Char, _Traits>& os, const ZZ<_n>& zRight)
{
	// буфер для десятичных / шестнадцатеричных разрядов word
	// log_10 256 = 2.4082..
	char buffer[sizeof(word) * 5 / 2 + 2];
	// шестнадцатеричный вывод?
	if ((os.flags() & std::ios_base::basefield) == std::ios_base::hex)
	{
		size_t pos = zRight.WordSize() - 1;
		while (pos > 0 && zRight.GetWord(pos) == 0) 
			pos--;
		bool waitfirst = true;
		if (os.flags() & std::ios_base::showbase)
			os << "0x";
		for (; pos != SIZE_MAX; pos--)
		{
			if (waitfirst)
				::sprintf(buffer, 
					sizeof(word) <= sizeof(unsigned) ? "%X" : "%lX", 
					zRight.GetWord(pos));
			else
				::sprintf(buffer, 
					sizeof(word) <= sizeof(unsigned) ? "%0*X" : "%0*lX", 
					int(sizeof(word) * 2), zRight.GetWord(pos));
			os << buffer;
			waitfirst = false;
		}
	}
	// десятичный вывод
	else
	{
		// деление
		word base = 10;
		int digits = 1;
		while (dword(base) * 10 == base * 10) 
			base *= 10, digits++;
		ZZ<_n> save(zRight);
		word symbol;
		std::vector<word> symbols;
		do
			save.Div(symbol = base),
			symbols.insert(symbols.begin(), symbol);
		while (save != word(0));
		// печать
		os << symbol;
		for (size_t pos = 1; pos < symbols.size(); pos++)
		{
			::sprintf(buffer, 
				sizeof(word) <= sizeof(unsigned) ? "%0*u" : "%0*lu",
				digits, symbols[pos]);
			os << buffer;
		}
	}
	return os;
}

//! Чтение из потока
/*! Число zRight читается из потока is.
	Перед чтением числа пропускаются пробелы и знаки табуляции. В зависимости
	от флагов форматирования число вводится в десятичной или шестнадцатеричной
	форме, с префиксом 0x для шестнадцатеричной формы или без (см. выше).
	Символы читаются от старших к младшим. Чтение невозможно, если в состоянии
	потока установлен флаг ошибки.
	Чтение прекращается, если
	1) превышена разрядность,
	2) достигнут конец потока,
	3) встретился недопустимый символ (не десятичная и не шестнадцатеричная
	цифра и не символ префикса).
	Во втором случае в состоянии потока будет установлен флаг ios_base::eofbit.
	Если не прочитано ни одной цифры, либо встретился недопустимый символ,
	то в состоянии потока будет установлен флаг ios_base::failbit,
	без снятия которого дальнейшее чтение из потока станет невозможно.
	При чтении из потока символа, после чтения которого превышается
	разрядность, прочитанный символ будет возвращен в поток.
	По окончании ввода число wRight будет содержать прочитанные символы,
	дополненные нулями. */
template<class _Char, class _Traits, size_t _n> inline
std::basic_istream<_Char, _Traits>& 
operator>>(std::basic_istream<_Char, _Traits>& is, ZZ<_n>& zRight)
{	
	// предварительно обнуляем число
	zRight.SetAll(0);
	// признак изменения
	bool changed = false;
	// захватываем поток и пропускаем пустые символы
	typename std::basic_istream<_Char, _Traits>::sentry s(is);
	// захват выполнен? 
	if (s)
	{
		// основание системы счисления
		word base = 10;
		// ожидается нулевой (не ожидается)? первый? второй? символы префикса
		word prefix = 0;
		if ((is.flags() & std::ios_base::basefield) == std::ios_base::hex)
		{
			base = 16;
			if (is.flags() & std::ios_base::showbase) 
				prefix = 1;
		}
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
			// ожидаем префикс?
			if (prefix == 1 && c != '0' || prefix == 2 && toupper(c) != 'X')
			{
				is.setstate(std::ios_base::failbit);
				break;
			}
			// ожидали префикс?
			if (prefix)
			{
				++prefix %= 3;
				continue;
			}
			// ожидаем цифру
			static char digits[] = "0123456789ABCDEF";
			char* hit =  std::strchr(digits, toupper(c));
			// подходящая цифра?
			if (hit && word(hit - digits) < base)
			{
				// учитываем цифру
				ZZ<_n + 4> tmp(zRight);
				(tmp *= base) += word(hit - digits);
				// нет переполнения?
				if (tmp < ZZ<_n + 4>(1).ShHi(_n))
				{
					changed = true;
					zRight = tmp;
					continue;
				}
			}
			else if (!changed)
				is.setstate(std::ios_base::failbit);
			// возвращаем неподходящий символ или символ переполнения
			is.rdbuf()->sputbackc(_Traits::to_char_type(c1));
			break;
		}
	}
	// чтение требовалось, но не удалось?
	else
		is.setstate(std::ios_base::failbit);
	return is;
}

} // namespace GF2

#endif // __GF2_ZZ