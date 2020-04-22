/*
*******************************************************************************
\file ww.h
\brief Binary words of arbitrary length
\project GF2 [algebra over GF(2)]
\created 2004.01.01
\version 2020.04.22
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file ww.h
\brief Двоичные слова

Модуль содержит описание и реализацию класса WW, поддерживающего манипуляции 
с двоичными словами произвольной конечной длины.
*******************************************************************************
*/

#ifndef __GF2_WW
#define __GF2_WW

#include "gf2/defs.h"
#include "gf2/env.h"
#include <cassert>
#include <iostream>

namespace GF2 {

/*!
*******************************************************************************
Класс WW

Поддерживает операции с двоичными словами заданной длины n > 0. 
Символы слов нумеруются от 0 до n - 1. 

Длина слова указывается при его создании: 
\code
	WW<n> w;
\endcode

Слово w разбивается на машинные слова типа word, которые хранятся во 
внутреннем массиве данных. Биты word нумеруются справа (от младших разрядов)
налево (к старшим). Биты дополнения в последнем слове представления всегда 
являются нулевыми. Например, при n = 13 и байтовых машинных словах символы 
\code
	w = w[0]w[1]...w[12] 
\endcode 
будут расположены в памяти следующим образом:
\code
	w[7]w[6]w[5]w[4]w[3]w[2]w[1]w[0] 000w[12]w[11]w[10]w[9]w[8].
\endcode

Внутренний класс Reference поддерживает ссылки на отдельные символы слова. 
Экземпляры класса Reference возвращаются в методе operator[]() и могут быть 
использованы в обеих частях оператора присваивания. Например,
\code
	WW<10> w;
	w[1] = 1; w[2] = w[1];
\endcode
При реализации класса Reference использован код STL std::bitset.

При проектировании была принята концепция WW как машинного слова заданной 
(какой угодно) длины. При этом для слов различных длин разрешено выполнять 
операции присваивания, сравнения, &=, |=, ^= и др. 
В этих операциях короткое rvalue дополняется нулями до длины lvalue,
либо, наоборот, длинное rvalue обрезается до длины lvalue.
В качестве rvalue кроме экземпляра WW может использоваться машинное слово 
типа word. Длина машинного слова полагается равной sizeof(word) * 8.
Такая концепция проектирования максимально близка к концепции связей между 
стандартными типами char, short, int, long. С другой стороны, мы немного 
отступаем от концепции слова как математического объекта.

Результатом двоичных операций w1 & w2, w1 | w2, w1 ^ w2 
является слово, длина которого определяется как максимум длин w1 и w2. 

При интерпретации w = w[0]w[1]... как машинного слова удобно считать,
что символ w[0] загружается в крайний справа разряд гипотетического
n-битового регистра, символ w[1] -- во второй справа разряд и так далее.
При такой загрузке стандартные обозначения для сдвигов << (влево)
и >> (вправо) трактуются корректно. Мы избегаем несколько
двусмысленной дихотомии вправо/влево и говорим о сдвигах в сторону
старших (с большими номерами) разрядов и в сторону младших
(с меньшими номерами). Перегруженные операторы-сдвиги operator<<() 
и operator>>() не являются рекомендуемыми: следует отдавать предпочтение 
методам ShHi() и ShLo().

При лексикографическом сравнении слова a и b считаются векторами
с неотрицательными целочисленными координатами и a > b, если самая 
правая ненулевая координата a - b положительна. Например,
\code
	111 > 011 > 101 > 001 > 110 > 010 > 100 > 000.
\endcode

Перед сравнением слов разной длины слово меньшей длины дополняется справа 
нулями. Может оказаться так, что слова разной длины будут признаны равными. 
Например, 100 == 10 == 1.

Большинство методов, выполняющих манипуляции над словом, возвращают ссылку
на само слово. Это позволяет использовать следующие синтаксические 
конструкции:
\code
	WW<7>().ShLo(4).Flip(1).RotHi(6) ^= w1;
\endcode

Ссылки не возвращают методы, критически влияющие на производительность:
Set(), SetWord() и др.

В машинных словах справа находятся младшие символы, а слева -- старшие.
В WW все наоборот: "младшие" в начале (слева), "старшие" в конце 
(справа). Данное различие не следует забывать при использовании
методов-сдвигов ShLo(), ShHi(), RotLo(), RotHi() и др.

\warning Неудачное поведение. В следующем фрагменте
\code
	WW<n> w;
	...
	std::cout << (w ^ 1);
\endcode
оператор w ^ 1 может быть интерпретирован двояко: как (word)w ^ 3 
и как w ^ WORD_1. Дело в том, что 1 является константой типа int, а не word. 
Явно указать на второй тип можно, написав (word)1 вместо 1.

\warning Реализация 
\code
	template<size_t _n, size_t _m> inline decltype(auto)
	operator^(const WW<_n>& wLeft, const WW<_m>& wRight)
	{
		return WW<std::max(_n, _m)>(wLeft) ^= wRight;
	}
\endcode
некорректна: decltype(auto) будет раскрываться как ссылка.
*******************************************************************************
*/

template<size_t _n> class WW
{
// длина
public:
	//! раскрытие длины
	static constexpr size_t n = _n;
	// запрет нулевой длины
	static_assert(_n > 0, "");

// поддержка ссылок на отдельные (!) биты слов
public:
	class Reference
	{
		friend class WW;
	private:
		WW* _ptr;
		size_t _pos;

	public:
		Reference& operator=(bool bVal)
		{
			_ptr->Set(_pos, bVal);
			return *this;
		}

		Reference& operator=(const Reference& ref)
		{
			_ptr->Set(_pos, bool(ref));
			return *this;
		}

		operator bool() const
		{
			return _ptr->Test(_pos);
		}

	private:
		Reference(WW& w, size_t pos)
		{
			assert(pos < _n);
			_ptr = &w;
			_pos = pos;
		}
	};

// данные
protected:
	// число машинных слов для хранения данных
	static constexpr size_t _wcount = (_n + B_PER_W - 1) / B_PER_W;	
	// число октетов для хранения данных
	static constexpr size_t _ocount = (_n + 7) / 8;
	// число неиспользуемых битов в последнем машинном слове
	static constexpr size_t _tcount = (B_PER_W - _n % B_PER_W) % B_PER_W;
	// машинные слова
	word _words[_wcount];

	//! Очистка битов дополнения
	/*! Очищаются неиспользуемые биты в последнем слове представления. */
	void Trim()
	{	
		if constexpr (_tcount != 0)
			(_words[_wcount - 1] <<= _tcount) >>= _tcount;
	}
	
	// открыть для WW<_m>
	template<size_t _m> friend class WW;

// базовые операции
public:
	//! Длина
	/*! Определяется длина слова. */
	static size_t Size()
	{
		return _n;
	}

	//! Длина в машинных словах
	/*! Определяется число машинных слов типа word для хранения слова.*/
	static size_t WordSize()
	{
		return _wcount;
	}

	//! Установка символа
	/*! Символу номер pos присваивается значение val.*/
	void Set(size_t pos, bool val)
	{
		assert(pos < _n);
		if (val) 
			_words[pos / B_PER_W] |= WORD_1 << pos % B_PER_W;
		else 
			_words[pos / B_PER_W] &= ~(WORD_1 << pos % B_PER_W);
	}

	//! Установка символов
	/*! Символы с номерами pos1 <= pos < pos2 заполняются значением val. */
	void Set(size_t pos1, size_t pos2, bool val)
	{	
		// номера слов представлений
		size_t wpos1 = pos1 / B_PER_W, wpos2 = pos2 / B_PER_W, pos;
		// в одном слове?
		if (wpos1 == wpos2)
		{
			for (pos = pos1; pos < pos2; Set(pos++, val));
			return;
		}
		// начинаем посередине слова представления?
		if (pos = pos1 % B_PER_W)
		{
			(_words[wpos1] <<= (B_PER_W - pos)) >>= B_PER_W - pos;
			if (val) 
				_words[wpos1] |= WORD_MAX << pos;
			wpos1++;
		}
		// заканчиваем посередине слова представления?
		if (pos = pos2 % B_PER_W)
		{
			(_words[wpos2] >>= pos) <<= pos;
			if (val) 
				_words[wpos2] |= WORD_MAX >> (B_PER_W - pos);
		}
		// заполняем полные слова
		for (pos = wpos1; pos < wpos2; _words[pos++] = (val ? WORD_MAX : 0));
	}

	//! Заполнение константой
	/*! Слово заполняется символом val. */
	void SetAll(bool val)
	{	
		for (size_t pos = 0; pos < _wcount; pos++)
			_words[pos] = val ? WORD_MAX : 0;
		Trim();
	}

	//! Заполнение нулями
	/*! Слово обнуляется. */
	void SetAllZero()
	{	
		for (size_t pos = 0; pos < _wcount; ++pos) 
			_words[pos] = 0;
	}

	//! Значение символа
	/*! Возвращается значение символа с номером pos. */
	bool Test(size_t pos) const
	{
		assert(pos < _n);
		return (_words[pos / B_PER_W] & (WORD_1 << pos % B_PER_W)) != 0;
	}

	//! Значение символа
	/*! Возвращается значение символа с номером pos. */
	bool operator[](size_t pos) const
	{
		return Test(pos);
	}

	//! Значение символа
	/*! Возвращается ссылка на символ с номером pos. */
	Reference operator[](size_t pos)
	{
		return Reference(*this, pos);
	}

	//! Инверсия символа
	/*! Инвертировать символ с номером pos. */
	WW& Flip(size_t pos)
	{	
		assert(pos < _n);
		_words[pos / B_PER_W] ^= WORD_1 << pos % B_PER_W;
		return *this;
	}

	//! Инверсия слова
	/*! Инвертировать все символы слова. */
	WW& FlipAll()
	{	
		for (size_t pos = 0; pos < _wcount; _words[pos++] ^= WORD_MAX);
		Trim();
		return *this;
	}

	//! Слово из одинаковых символов?
	/*! Проверяется, что слово состоит из символов val. */
	bool IsAll(bool val) const
	{
		// проверить все слова представления, кроме последнего
		word w = val ? WORD_MAX : 0;
		size_t pos = 0;
        for (; pos + 1 < _wcount; pos++)
			if (_words[pos] != w) 
				return false;
		// проверить последнее слово представления
		return _words[pos] == (w << _tcount >> _tcount);
	}

	//! Нулевое слово?
	/*! Проверяется, что все символы слова нулевые. */
	bool IsAllZero() const
	{
		assert(_n > 0);
		for (size_t pos = 0; pos < _wcount; pos++)
			if (_words[pos] != 0)
				return false;
		return true;
	}

	//! Обратный порядок символов
	/*! Символы слова перезаписываются в обратном порядке. */
	WW& Reverse()
	{	
		for (size_t start = 0, end = _n; start + 1 < end; ++start, --end)
		{
			bool save = Test(start);
			Set(start, Test(end - 1));
			Set(end - 1, save);
		}
		return *this;
	}

	//! Возврат слова представления
	/*! Определяется слово представления с номером pos. */
	word GetWord(size_t pos) const
	{	
		assert(pos < _wcount);
		return _words[pos];
	}

	//! Возврат октета представления
	/*! Определяется октет представления с номером pos. */ 
	octet GetOctet(size_t pos) const
	{	
		assert(pos < _ocount);
		return octet(_words[pos / O_PER_W] >> pos % O_PER_W * 8);
	}

	//! Устанавка слова представления
	/*! Слово представления с номером pos устанавливается равным val.*/
	void SetWord(size_t pos, word val)
	{	
		assert(pos < _wcount);
		_words[pos] = val;
		if (pos == _wcount - 1) 
			Trim();
	}

	//! Установка октета представления
	/*! Октет представления с номером pos устанавливается равным val. */
	void SetOctet(size_t pos, octet val)
	{	
		assert(pos < _ocount);
		_words[pos / O_PER_W] &= ~(word(255) << pos % O_PER_W * 8);
		_words[pos / O_PER_W] |= word(val) << pos % O_PER_W * 8;
	}

	//! Обмен
	/*! Производится обмен символами со словом wRight. */
	void Swap(WW& wRight)
	{
		for (size_t pos = 0; pos < _wcount; ++pos)
		{
			word w = wRight._words[pos];
			wRight._words[pos] = _words[pos];
			_words[pos] = w;
		}
	}

	//! Вес
	/*! Определяется вес (число ненулевых символов) слова. */
	size_t Weight() const
	{	
		static unsigned char _bitsperoctet[256] = 
		{
			0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
			1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
		};
		size_t weight = 0;
		for (size_t pos = 0; pos < _ocount; pos++)
			weight += _bitsperoctet[GetOctet(pos)];
		return weight;
	}

	//! Бит четности
	/*! Определяется бит четности (сумма символов mod 2). */
	bool Parity() const
	{	
		return bool(Weight() & 1u);
	}

// манипуляции с наборами символов
public:
	//! Сдвиг в младшую сторону
	/*! Сдвиг символов слова в младшую сторону с записью нулей в старшую часть:
		\code
			w[0]w[1]...w[n-1] -> w[shift]w[shift + 1]...w[n-1]00...0.
		\endcode */
	WW& ShLo(size_t shift)
	{	
		if (shift < _n)
		{
			size_t wshift = shift / B_PER_W, pos;
			// величина сдвига не кратна длине слова?
			if (shift %= B_PER_W)
			{
				// сдвиг всех слов, кроме последнего
				for (pos = 0; pos + wshift + 1 < _wcount; pos++)
					_words[pos] = (_words[pos + wshift] >> shift) |
						(_words[pos + wshift + 1] << (B_PER_W - shift));
				// последнее слово
				_words[pos] = _words[pos + wshift] >> shift, ++pos;
			}
			// величина сдвига кратна длине слова
			else for (pos = 0; pos + wshift < _wcount; ++pos)
				_words[pos] = _words[pos + wshift];
			// обнуление правых слов
			for (; pos < _wcount; _words[pos++] = 0);
		}
		else 
			SetAllZero();
		return *this;
	}

	//! Сдвиг в старшую сторону
	/*! Сдвиг символов слова в старшую сторону с записью нулей в младшую часть:
		\code
			w[0]w[1]...w[n-1] -> 00...0w[0]w[1]...w[n-1-shift].
		\endcode */
	WW& ShHi(size_t shift)
	{	
		if (shift < _n)
		{
			size_t wshift = shift / B_PER_W, pos;
			// величина сдвига не кратна длине слова?
			if (shift %= B_PER_W)
			{
				// сдвиг всех слов, кроме первого
				for (pos = _wcount - 1; pos > wshift; --pos)
					_words[pos] = (_words[pos - wshift] << shift) |
						(_words[pos - wshift - 1] >> (B_PER_W - shift));
				// первое слово
				_words[pos] = _words[pos - wshift] << shift, --pos;
			}
			// величина сдвига кратна длине слова
			else for (pos = _wcount - 1; pos + 1 > wshift; --pos)
					_words[pos] = _words[pos - wshift];
			// очистка последнего слова
			Trim();
			// обнуление левых слов
			for (; pos != SIZE_MAX; _words[pos--] = 0);
		}
		else SetAllZero();
		return *this;
	}

	//! Циклический сдвиг в младшую сторону
	/*! Циклический сдвиг символов слова в младшую сторону. */
	WW& RotLo(size_t shift)
	{	
		shift %= _n;
		return ShLo(shift) |= WW(*this).ShHi(_n - shift);
	}

	//! Циклический сдвиг в старшую сторону
	/*! Циклический сдвиг символов слова в старшую сторону. */
	WW& RotHi(size_t shift)
	{	
		shift %= _n;
		return ShHi(shift) |= WW(*this).ShLo(_n - shift);
	}

	//! Выбор младшей части
	/*! Младшие _m <= _n символов слова записываются в w.
		\return Ссылка на w. */
	template<size_t _m>
	WW<_m>& GetLo(WW<_m>& w) const
	{
		static_assert(_m <= _n, "");
		for (size_t pos = 0; pos < w._wcount; ++pos)
			w._words[pos] = _words[pos];
		w.Trim();
		return w;
	}

	//! Выбор младшей части
	/*! Возвращаются младшие _m <= _n символов слова. */
	template<size_t _m>
	WW<_m> GetLo() const
	{
		WW<_m> w;
		return GetLo(w);
	}

	//! Установка младшей части
	/*! Младшие _m символов слова устанавливаются по w. 
		\return Ссылка на само слово. */
	template<size_t _m>
	WW& SetLo(const WW<_m>& w)
	{
		static_assert(_m <= _n, "");
		size_t pos = 0;
		for (; pos + 1 < w._wcount; ++pos)
			_words[pos] = w._words[pos];
		// неполное последнее слово w?
		if constexpr (WW<_m>::_tcount != 0)
			_words[pos] = w._words[pos] | 
				(_words[pos] & ~((WORD_HI >> (w._tcount - 1)) - WORD_1));
		else
			_words[pos] = w._words[pos];
		return *this;
	}

	//! Выбор старшей части
	/*! Старшие _m <= _n символов слова записываются в w.
		\return Ссылка на w. */
	template<size_t _m>
	WW<_m>& GetHi(WW<_m>& w) const
	{
		static_assert(_m <= _n, "");
		// первое слово, в котором начинается правая часть
		size_t start = (_n - _m) / B_PER_W;
		// правая часть начинается посередине слова (со смещением offset)?
		size_t pos;
		if (size_t offset = (_n - _m) % B_PER_W)
		{
			// объединять биты двух последовательных слов
			for (pos = 0; pos + start + 1 < _wcount; pos++)
				w._words[pos] = (_words[pos + start] >> offset) | 
					(_words[pos + start + 1] << (B_PER_W - offset));
			// использовать биты последнего слова
			if (pos < w._wcount)
				w._words[pos] = _words[pos + start] >> offset;
			w.Trim();
		}
		else
			for (pos = 0; pos < w._wcount; ++pos)
				w._words[pos] = _words[pos + start];
		return w;
	}

	//! Выбор старшей части
	/*! Возвращаются старшие _m <= _n символов слова. */
	template<size_t _m>
	WW<_m> GetHi() const
	{
		WW<_m> w;
		return GetHi(w);
	}

	//! Установка старшей части
	/*! Старшие _m <= _n символов слова устанавливаются по w.
		\return ссылка на само слово. */
	template<size_t _m>
	WW& SetHi(const WW<_m>& w)
	{
		static_assert(_m <= _n, "");
		// первое слово, в котором начинается правая часть
		size_t start = (_n - _m) / B_PER_W;
		// правая часть начинается посередине слова (со смещением offset)?
		if (size_t offset = (_n - _m) % B_PER_W)
		{
			// очистить старшие B_PER_W - offset битов _words[start]
			(_words[start] <<= B_PER_W - offset) >>= B_PER_W - offset;
			// и записать в них младшие биты первого слова w
			_words[start] |= w._words[0] << offset;
			// далее объединять биты двух последовательных слов w
			size_t pos;
			for (pos = 1; pos < w._wcount; ++pos)
				_words[pos + start] = (w._words[pos] << offset) |
					(w._words[pos - 1] >> (B_PER_W - offset));
			// обработать остатки
			if (pos + start < _wcount)
				_words[pos + start] = w._words[pos - 1] >> (B_PER_W - offset);
		}
		else 
			for (size_t pos = 0; pos < w._wcount; ++pos)
				_words[pos + start] = w._words[pos];
		Trim();
		return *this;
	}

	//! Упаковка
	/*! Удаляются (со сдвигом в младшую часть) символы с индексами pos 
		такими,	что wMask[pos] == 0. */
	WW& Pack(const WW& wMask)
	{
		assert(this != &wMask);
		size_t pos = 0, posMask = 0;
		for (posMask = 0; posMask < _n; ++posMask)
			if (wMask.Test(posMask))
				Set(pos++, Test(posMask));
		// добиваем нулями
		for (; pos < _n && pos % B_PER_W; Set(pos++, 0));
		for (; pos < _wcount; _words[pos++] = 0);
		return *this;
	}

	//! Распаковка
	/*! В слово вставляются нулевые символы с индексами pos такими,
		что wMask[pos] == 0. */
	WW& Unpack(const WW& wMask)
	{
		assert(this != &wMask);
		size_t pos = wMask.Weight(), posMask = _n;
		while (posMask--)
			Set(posMask, wMask.Test(posMask) ? Test(--pos) : 0);
		return *this;
	}

	//! Перестановка
	/*! Слово w[0]w[1]...w[n-1]	заменяется на слово u[0]u[1]...u[n-1],
		в котором u[i] == w[pi[i]], если pi[i] != -1 и u[i] == 0 
		в противном случае. */
	WW& Permute(const size_t pi[_n])
	{	
		WW<_n> temp;
		for (size_t pos = 0; pos < _n; ++pos)
			temp.Set(pos, pi[pos] == SIZE_MAX ? 0 : Test(pi[pos]));
		return operator=(temp);
	}

// лексикографический порядок
public:
	//! Сравнение
	/*! Выполняется лексикографическое сравнение со словом wRight. 
		\return -1 (<), 0 (=), 1 (>). */
	int Compare(const WW& wRight) const
	{
		for (size_t pos = _wcount - 1; pos != SIZE_MAX; --pos)
			if (_words[pos] > wRight._words[pos]) 
				return 1;
			else if (_words[pos] < wRight._words[pos]) 
				return -1;
		return 0;
	}

	//! Сравнение
	/*! Выполняется лексикографическое сравнение со словом wRight 
		другой длины. 
		\return -1 (<), 0 (=), 1 (>). */
	template<size_t _m>
	int Compare(const WW<_m>& wRight) const
	{
		size_t pos;
		if (_n > _m) 
		{
			for (pos = _wcount - 1; pos != wRight._wcount - 1; --pos)
				if (_words[pos] != 0) 
					return 1;
		}
		else for (pos = wRight._wcount - 1; pos != _wcount - 1; --pos)
			if (wRight._words[pos] != 0) 
				return -1;
		for (; pos != SIZE_MAX; --pos)
			if (_words[pos] > wRight._words[pos]) 
				return 1;
			else if (_words[pos] < wRight._words[pos]) 
				return -1;
		return 0;
	}

	//! Первое слово
	/*! Определяется первое в лексикографическом порядке слово с заданным 
		весом weight. */
	void First(size_t weight = 0)
	{	
		assert(weight <= _n);
		Set(0, weight, 1);
		Set(weight, _n, 0);
	}

	//! Последнее слово
	/*! Определяется последнее в лексикографическом порядке слово с заданным
		весом weight. */
	void Last(size_t weight = _n)
	{	
		assert(weight <= _n);
		Set(0, _n - weight, 0);
		Set(_n - weight, _n, 1);
	}

	//! Следующее слово
	/*! Определяется следующее в лексикографическом порядке слово 
		(той же степени, если установлен флаг saveWeight).
		Если слово является последним, то будет построено первое слово.
		\return true, если построено следующее слово и false,
		если возвратились к первому. */
	bool Next(bool saveWeight = false)
	{	
		size_t pos = 0;
		if (!saveWeight)
		{
			// инкремент слова-как-числа
			for (; pos < _wcount && ++_words[pos] == 0; ++pos);
			// перенос не попал в последнее слово представления?
			if (pos + 1 < _wcount) 
				return true;
			// прошли последнее слово?
			if (pos == _wcount) 
				return false;
			// остановились в последнем слове
			Trim(); 
			return _words[pos] != 0;
		}
		// ищем начало серии из единиц
		for (; pos < _n && !Test(pos); ++pos);
		// единиц нет?
		if (pos == _n) 
			return false;
		// ищем окончание серии
		size_t end = pos + 1;
		for (; end < _n && Test(end); ++end);
		// серия справа?
		if (end == _n)
		{
			// возвращаемся к первому слову с данным весом
			First(end - pos);
			return false;
		}
		// разрываем серию
		Set(0, end - pos - 1, 1); 
		Set(end - pos - 1, end, 0);
		Set(end, 1);
		return true;
	}

	//! Предыдущее слово
	/*! Определяется предыдущее в лексикографическом порядке слово 
		(той же степени, если установлен флаг saveWeight).
		Если слово является первым, то будет построено последнее слово.
		\return true, если построено предыдущее слово и false,
		если вовратились к последнему. */
	bool Prev(bool saveWeight = false)
	{
		size_t pos = 0;
		if (!saveWeight)
		{
			// декремент слова-как-числа
			for (; pos < _wcount && --_words[pos] == SIZE_MAX; ++pos);
			// перенос не попал или остался в последнем слове представления?
			if (pos < _wcount) 
				return true;
			Trim();
			return false;
		}
		// ищем первую единицу
		for (; pos < _n && !Test(pos); ++pos);
		// единиц нет?
		if (pos == _n) 
			return false;
		// начинается с нулей?
		if (pos > 0)
		{
			// передвигаем единицу влево
			Set(pos - 1, 1), Set(pos, 0);
			return true;
		}
		// ищем окончание серии из единиц
		size_t end = 1;
		for (; end < _n && Test(end); ++end);
		// ищем следующую единицу
		for (pos = end; pos < _n && !Test(pos); ++pos);
		// серия слева: 1^end 00...0?
		if (pos == _n)
		{
			// возвращаемся к последнему слову с данным весом
			Last(end);
			return false;
		}
		// сдвигаем 1^{end}0^{pos-end}1... -> 0^{pos-end-1}1^{end+1}0...
		Set(0, pos - end - 1, 0), Set(pos - end - 1, pos, 1), Set(pos, 0);
		return true;
	}

	//! Задать наудачу
	/*! Генерация случайного слова. */
	WW& Rand()
	{
		Env::RandMem(_words, _ocount);
		Trim();
		return *this;
	}

// операции
public:
	//! Присваивание
	/*! Присваивание слову значения-слова wRight. */
	WW& operator=(const WW& wRight)
	{
		for (size_t pos = 0; pos < _wcount; ++pos)
			_words[pos] = wRight._words[pos];
		return *this;
	}

	//! Присваивание
	/*! Присваивание слову значения-слова wRight другой размерности. */
	template<size_t _m>
	WW& operator=(const WW<_m>& wRight)
	{
		size_t pos = 0;
		for (; pos < std::min(_wcount, wRight._wcount); ++pos)
			_words[pos] = wRight._words[pos];
		if constexpr (_n < _m) 
			Trim();
		else 
			for (; pos < _wcount; _words[pos++] = 0);
		return *this;
	}

	//! Присваивание
	/*! Присваивание слову значения-машинного слова wRight. */
	WW& operator=(word wRight)
	{	
		_words[0] = wRight;
		for (size_t pos = 1; pos < _wcount; pos++)
			_words[pos] = 0;
		Trim();
		return *this;
	}

	//! Приведение к машинному слову
	/*! Возвращается первое слово представления. */
	operator word() const
	{
		return _words[0];
	}

	//! Равенство
	/*! Проверяется равенство слову wRight. */
	template<size_t _m>
	bool operator==(const WW<_m>& wRight) const
	{	
		return Compare(wRight) == 0;
	}

	//! Равенство
	/*! Проверяется равенство машинному слову wRight. */
	bool operator==(word wRight) const
	{	
		if (_words[0] != wRight)
			return false;
		for (size_t pos = 1; pos < _wcount; ++pos)
			if (_words[pos] != 0)
				return false;
		return true;
	}

	//! Неравенство
	/*! Проверяется неравенство слову wRight. */
	template<size_t _m>
	bool operator!=(const WW<_m>& wRight) const
	{	
		return Compare(wRight) != 0;
	}

	//! Неравенство
	/*! Проверяется неравенство машинному слову wRight. */
	bool operator!=(word wRight) const
	{	
		return !operator==(wRight);
	}

	//! Меньше?
	/*! Проверяется, что слово лексикографические меньше wRight. */
	template<size_t _m>
	bool operator<(const WW<_m>& wRight) const
	{	
		return Compare(wRight) < 0;
	}

	//! Меньше?
	/*! Проверяется, что слово лексикографически меньше машинного 
		слова wRight. */
	bool operator<(word wRight) const
	{	
		if (_words[0] >= wRight)
			return false;
		for (size_t pos = 1; pos < _wcount; pos++)
			if (_words[pos] > 0)
				return false;
		return true;
	}

	//! Не больше?
	/*! Проверяется, что слово лексикографические не больше wRight. */
	template<size_t _m>
	bool operator<=(const WW<_m>& wRight) const
	{	
		return Compare(wRight) <= 0;
	}

	//! Не больше?
	/*! Проверяется, что слово не больше машинного слова wRight. */
	bool operator<=(word wRight) const
	{	
		if (_words[0] > wRight)
			return false;
		for (size_t pos = 1; pos < _wcount; ++pos)
			if (_words[pos] > 0)
				return false;
		return true;
	}

	//! Больше?
	/*! Проверяется, что слово лексикографические больше wRight. */
	template<size_t _m>
	bool operator>(const WW<_m>& wRight) const
	{	
		return Compare(wRight) > 0;
	}

	//! Больше?
	/*! Проверяется, что слово больше машинного слова wRight. */
	bool operator>(word wRight) const
	{	
		return !operator<=(wRight);
	}

	//! Не меньше?
	/*! Проверяется, что слово лексикографические не менььше wRight. */
	template<size_t _m>
	bool operator>=(const WW<_m>& wRight) const
	{	
		return Compare(wRight) >= 0;
	}

	//! Не меньше?
	/*! Проверяется, что слово не меньше машинного слова wRight. */
	bool operator>=(word wRight) const
	{	
		return !operator<(wRight);
	}

	//! Инверсия
	/*! Символы слова инвертируются. */
	WW operator~() const
	{	
		return WW(*this).FlipAll();
	}

	//! AND
	/*! Выполняется логическое умножение символов на 
		соответствующие символы слова wRight. */
	WW& operator&=(const WW& wRight)
	{	
		for (size_t pos = 0; pos < _wcount; ++pos)
			_words[pos] &= wRight._words[pos];
		return *this;
	}

	//! AND
	/*! Выполняется логическое умножение символов на 
		соответствующие символы слова wRight другой длины. */
	template<size_t _m>
	WW& operator&=(const WW<_m>& wRight)
	{	
		for (size_t pos = 0; pos < std::min(_wcount, wRight._wcount); ++pos)
			_words[pos] &= wRight._words[pos];
		if constexpr (_m < _n)
			for (size_t pos = wRight._wcount; pos < _wcount; _words[pos++] = 0);
		return *this;
	}

	//! AND
	/*! Выполняется логическое умножение символов на 
		соответствующие символы машинного слова wRight. */
	WW& operator&=(word wRight)
	{	
		_words[0] &= wRight;
		for (size_t pos = 1; pos < _wcount; _words[pos++] = 0);
		return *this;
	}

	//! OR
	/*! Выполняется логическое сложение символов с 
		соответствующими символами слова wRight. */
	WW& operator|=(const WW& wRight)
	{	
		for (size_t pos = 0; pos < _wcount; ++pos)
			_words[pos] |= wRight._words[pos];
		return *this;
	}

	//! OR
	/*! Выполняется логическое сложение символов с
		соответствующими символами слова wRight другой длины. */
	template<size_t _m>
	WW& operator|=(const WW<_m>& wRight)
	{	
		for (size_t pos = 0; pos < std::min(_wcount, wRight._wcount); ++pos)
			_words[pos] |= wRight._words[pos];
		if constexpr (_n < _m)
			Trim();
		return *this;
	}

	//! OR
	/*! Выполняется логическое сложение символов с 
		соответствующими символами машинного слова wRight. */
	WW& operator|=(word wRight)
	{	
		_words[0] |= wRight;
		if constexpr (_wcount == 1)
			Trim();
		return *this;
	}

	//! XOR
	/*! Выполняется исключающее логическое сложение символов с 
		соответствующими символами слова wRight. */
	WW& operator^=(const WW& wRight)
	{	
		for (size_t pos = 0; pos < _wcount; ++pos)
			_words[pos] ^= wRight._words[pos];
		return *this;
	}

	//! XOR
	/*! Выполняется исключающее логическое сложение символов с
		соответствующими символами слова wRight другой длины. */
	template<size_t _m>
	WW& operator^=(const WW<_m>& wRight)
	{	
		for (size_t pos = 0; pos < std::min(_wcount, wRight._wcount); ++pos)
			_words[pos] ^= wRight._words[pos];
		if constexpr (_n < _m)
			Trim();
		return *this;
	}

	//! XOR
	/*! Выполняется исключающее логическое сложение символов с 
		соответствующими символами машинного слова wRight. */
	WW& operator^=(word wRight)
	{	
		_words[0] ^= wRight;
		if constexpr (_wcount == 1)
			Trim();
		return *this;
	}

	//! Сдвиг в младшую сторону
	/*! Выполняется сдвиг символов слова в младшую сторону. */
	WW& operator>>=(size_t shift)
	{	
		return ShLo(shift);
	}

	//! Сдвиг в младшую сторону
	/*! Возвращается слово со сдвинутыми в младшую сторону символами. */
	WW operator>>(size_t shift) const
	{	
		return WW(*this).ShLo(shift);
	}

	//! Сдвиг в старшую сторону
	/*! Выполняется сдвиг символов слова в старшую сторону. */
	WW& operator<<=(size_t shift)
	{	
		return ShHi(shift);
	}

	//! Сдвиг в старшую сторону
	/*! Возвращается слово со сдвинутыми в старшую сторону символами. */
	WW operator<<(size_t shift) const
	{	
		return WW(*this).ShHi(shift);
	}


// конструкторы
public:
	//! Конструктор по умолчанию
	/*! Создается нулевое слово. */
	WW()
	{
		SetAllZero();
	}
	
	//! Конструктор по машинному слову
	/*! Создается копия машинного слова wRight 
		(возможно, с потерей старших битов wRight или, наоборот, 
		с добавлением нулевых символов). */
	WW(word wRight)
	{
		_words[0] = wRight;
		if constexpr (_wcount == 1)
			Trim();
		else
			for (size_t pos = 1; pos < _wcount; _words[pos++] = 0);
	}

	//! Конструктор копирования
	/*! Создается копия слова wRight. */
	WW(const WW& wRight)
	{	
		for (size_t pos = 0; pos < _wcount; ++pos)
			_words[pos] = wRight._words[pos];
	}

	//! Конструктор копирования
	/*! Создается копия слова wRight другой длины. */
	template<size_t _m> 
	WW(const WW<_m>& wRight)
	{
		size_t pos = 0;
		for (; pos < std::min(_wcount, wRight._wcount); ++pos)
			_words[pos] = wRight._words[pos];
		if constexpr (_n < _m)
			Trim();
		else 
			for (; pos < _wcount; _words[pos++] = 0);
	}
};

//! Равенство
/*! Проверяется равенство машинного слова wLeft и слова wRight. */
template<size_t _n> inline bool
operator==(word wLeft, const WW<_n>& wRight)
{
	return wRight == wLeft;
}

//! Неравенство
/*! Проверяется неравенство машинного слова wLeft и слова wRight. */
template<size_t _n> inline bool
operator!=(word wLeft, const WW<_n>& wRight)
{
	return wRight != wLeft;
}

//! Меньше?
/*! Проверяется, что машинное слово wLeft меньше слова wRight. */
template<size_t _n> inline bool
operator<(word wLeft, const WW<_n>& wRight)
{
	return wRight > wLeft;
}

//! Не больше?
/*! Проверяется, что машинное слово wLeft не больше слова wRight. */
template<size_t _n> inline bool
operator<=(word wLeft, const WW<_n>& wRight)
{
	return wRight >= wLeft;
}

//! Больше?
/*! Проверяется, что машинное слово wLeft больше слова wRight. */
template<size_t _n> inline bool
operator>(word wLeft, const WW<_n>& wRight)
{
	return wRight < wLeft;
}

//! Не меньше?
/*! Проверяется, что машинное слово wLeft не меньше слова wRight. */
template<size_t _n> inline bool
operator>=(word wLeft, const WW<_n>& wRight)
{
	return wRight <= wLeft;
}

//! AND
/*! Определяется слово wLeft & wRight. */
template<size_t _n, size_t _m> inline decltype(auto)
operator&(const WW<_n>& wLeft, const WW<_m>& wRight)
{	
	WW<std::max(_n, _m)> w(wLeft);
	w &= wRight;
	return w;
}

//! AND
/*! Определяется слово wLeft & wRight (wRight -- машинное слово). */
template<size_t _n> inline decltype(auto)
operator&(const WW<_n>& wLeft, word wRight)
{	
	WW<std::max(_n, sizeof(word) * 8)> w(wLeft);
	w &= wRight;
	return w;
}

//! AND
/*! Определяется слово wLeft & wRight (wLeft -- машинное слово). */
template<size_t _n> inline decltype(auto)
operator&(word wLeft, const WW<_n>& wRight)
{	
	WW<std::max(_n, sizeof(word) * 8)> w(wRight);
	w &= wLeft;
	return w;
}

//! OR
/*! Определяется слово wLeft | wRight. */
template<size_t _n, size_t _m> inline decltype(auto)
operator|(const WW<_n>& wLeft, const WW<_m>& wRight)
{	
	WW<std::max(_n, _m)> w(wLeft);
	w |= wRight;
	return w;
}

//! OR
/*! Определяется слово wLeft | wRight (wRight -- машинное слово). */
template<size_t _n> inline decltype(auto)
operator|(const WW<_n>& wLeft, word wRight)
{	
	WW<std::max(_n, sizeof(word) * 8)> w(wLeft);
	w |= wRight;
	return w;
}

//! OR
/*! Определяется слово wLeft | wRight (wLeft -- машинное слово). */
template<size_t _n> inline decltype(auto)
operator|(word wLeft, const WW<_n>& wRight)
{	
	WW<std::max(_n, sizeof(word) * 8)> w(wRight);
	w |= wLeft;
	return w;
}

//! XOR
/*! Определяется слово wLeft ^ wRight. */
template<size_t _n, size_t _m> inline decltype(auto)
operator^(const WW<_n>& wLeft, const WW<_m>& wRight)
{	
	WW<std::max(_n, _m)> w(wLeft);
	w ^= wRight;
	return w;
}

//! XOR
/*! Определяется слово wLeft ^ wRight (wRight -- машинное слово). */
template<size_t _n> inline decltype(auto)
operator^(const WW<_n>& wLeft, word wRight)
{	
	WW<std::max(_n, sizeof(word) * 8)> w(wLeft);
	w ^= wRight;
	return w;
}

//! XOR
/*! Определяется слово wLeft ^ wRight (wLeft -- машинное слово). */
template<size_t _n> inline decltype(auto)
operator^(word wLeft, const WW<_n>& wRight)
{	
	WW<std::max(_n, sizeof(word) * 8)> w(wRight);
	w ^= wLeft;
	return w;
}

//! Конкатенация слов
/*! Слова wLeft и wRight конкатенируются. */
template<size_t _n, size_t _m> inline decltype(auto)
Concat(const WW<_n>& wLeft, const WW<_m>& wRight)
{	
	WW<_n + _m> w(wLeft);
	w.SetHi(wRight);
	return w;
}

//! Конкатенация слов
/*! Слова wLeft и wRight конкатенируются. 
	\remark Запись a || b соответствует устоявшимся
	математическим обозначениям. */
template<size_t _n1, size_t _n2> inline decltype(auto)
operator||(const WW<_n1>& wLeft, const WW<_n2>& wRight)
{	
	WW<_n1 + _n2> w(wLeft);
	w.SetHi(wRight);
	return w;
}

//! Вывод в поток
/*! Слово wRight выводится в поток os. */
template<class _Char, class _Traits, size_t _n> inline 
std::basic_ostream<_Char, _Traits>& 
operator<<(std::basic_ostream<_Char, _Traits>& os, const WW<_n>& wRight)
{
	for (size_t pos = 0; pos < _n; pos++)
		os << (wRight.Test(pos) ? "1" : "0");
	return os;
}

//! Ввод из потока
/*! Слово wRight читается из потока is. 
	\remark Перед чтением слова пропускаются пробелы и знаки табуляции.
	Чтение невозможно, если в состоянии потока установлен флаг ошибки.
	\remark Чтение слова прекращается, если 
	1) прочитаны все символы,
	2) достигнут конец потока,
	3) встретился недопустимый символ (не 0 или 1). 
	Во втором случае в состоянии потока будет установлен флаг ios_base::eofbit.
	Если не прочитано ни одного символа непустого слова, то в состоянии потока
	будет установлен флаг ios_base::failbit, без снятия которого дальнейшее 
	чтение из потока невозможно. 
	При чтении из потока недопустимого символа он будет возвращен в поток.
	По окончании ввода слово wRight будет содержать прочитанные символы, 
	дополненные нулями. */
template<class _Char, class _Traits, size_t _n> inline
std::basic_istream<_Char, _Traits>& 
operator>>(std::basic_istream<_Char, _Traits>& is, WW<_n>& wRight)
{	
	// предварительно обнуляем слово
	wRight.SetAll(0);
	// захватываем поток и пропускаем пустые символы
	typename std::basic_istream<_Char, _Traits>::sentry s(is);
	// захват выполнен? 
	if (s)
		// пробегаем символы слова
		for (size_t pos = 0; pos < _n; ++pos) 
		{
			// читаем символ из потока
			auto c1 = is.rdbuf()->sbumpc();
			// конец файла?
			if (_Traits::eq_int_type(c1, _Traits::eof()))
			{
				is.setstate(std::ios_base::eofbit);
				// ничего не прочитано?
				if (pos == 0) 
					is.setstate(std::ios_base::failbit);
				break;
			}
			else 
			{
				char c = is.narrow(_Traits::to_char_type(c1), '*');
				// подходящий символ?
				if (c == '0' || c == '1')
					wRight.Set(pos, c == '0' ? 0 : 1);
		        else 
				{
					// возвращаем недопустимый символ в поток
					is.rdbuf()->sputbackc(_Traits::to_char_type(c1));
					if (pos == 0)
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

#endif // __GF2_WW