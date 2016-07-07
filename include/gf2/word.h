/*
*******************************************************************************
\file word.h
\brief Words in GF(2)
\project GF2 [GF(2) algebra library]
\author (С) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2004.01.01
\version 2016.07.06
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file word.h
\brief Двоичные слова

Модуль содержит описание и реализацию класса Word, поддерживающего манипуляции 
с двоичными словами произвольной конечной длины.
*******************************************************************************
*/

#ifndef __GF2_WORD
#define __GF2_WORD

#include "gf2/defs.h"
#include "gf2/env.h"
#include <cassert>
#include <iostream>

namespace GF2{

/*!
*******************************************************************************
Класс Word

-#  Поддерживает операции с двоичными словами заданной длины n. 
	Символы слов нумеруются от 0 до n - 1. 
	Длина слова указывается при его создании: 
	\code
	Word<n> w;
	\endcode
-#  Слово w разбивается на машинные слова типа word, которые хранятся во 
	внутреннем массиве данных. Символы word нумеруются справа 
	(от младших разрядов) налево (к старшим). 
	Биты дополнения в последнем слове представления всегда являются нулевыми.
	Например, при n = 13 и байтовых машинных словах символы 
	\code
	w = w[0]w[1]...w[12] 
	\endcode 
	будут расположены в памяти следующим образом:
	\code
	w[7]w[6]w[5]w[4]w[3]w[2]w[1]w[0] 000w[12]w[11]w[10]w[9]w[8].
	\endcode
-#  Внутренний класс Reference поддерживает ссылки на отдельные
    символы слова. Экземпляры класса Reference возвращаются в методе
    operator[]() и могут быть использованы в обеих частях оператора 
	присваивания. При реализации класса Reference использован 
    код STL std::bitset.
-#	При проектировании была принята концепция Word как машинного 
	слова заданной (какой угодно) длины. При этом для слов различных длин 
	разрешено выполнять операции присваивания, сравнения, &=, |=, ^= и др.
	В таких операциях короткое rvalue дополняется нулями до длины lvalue,
	либо, наоборот, длинное rvalue обрезается до длины lvalue.
	В качестве rvalue кроме экземпляра Word может использоваться 
	машинное слово типа word. Длина машинного слова полагается равной 
	sizeof(word) * 8.
	Такая концепция проектирования максимально близка к концепции 
	связей между стандартными типами char, short, int, long.
	С другой стороны, мы немного отступаем от концепции слова как 
	математического объекта.
-#	Результатом двоичных операций w1 & w2, w1 | w2, w1 ^ w2 
	является слово, длина которого определяется как максимум длин w1 и w2. 
-#	При лексикографическом сравнении слова a и b считаются векторами
	с неотрицательными целочисленными координатами и a > b, если самая правая
	ненулевая координата a - b положительна. Например,
	111 > 011 > 101 > 001 > 110 > 010 > 100 > 000.
	Перед сравнением слов разной длины слово меньшей длины 
	дополняется справа нулями. Может оказаться так, что слова разной длины 
	будут признаны равными. Например, 100 == 10 == 1.
-#	Большинство методов, выполняющих манипуляции над словом, возвращают ссылку
	на само слово. Это позволяет использовать следующие синтаксические
	конструкции: 
	\code
	Word<7>().Shl(4).Flip(1).Rotl(6) ^= w1;
	\endcode
	Ссылки не возвращают методы, критически влияющие на производительность:
	Set(), SetWord() и др.
-#	В машинных словах справа находятся младшие символы, а слева -- старшие.
	В Word все наоборот: "младшие" в начале (слева), "старшие" в конце 
	(справа). Данное различие не следует забывать при использовании
	методов-сдвигов Shl(), Shr(), Rotl(), Rotr() и др.
-#	Для математической завершенности разрешается определять объекты типа 
	Word<0> (пустые слова). Однако, операции по их изменению (напр., сдвиги)
	в большинстве случаев приведут к исключениям.
-#	Неудачное поведение. В следующем фрагменте
	\code
	Word<n> w;
	...
	std::cout << (w ^ 1);
	\endcode
	оператор w ^ 1 может быть интерпретирован двояко: 
	как (word)w ^ 1 и как w ^ (word)1. Дело в том, что 1 
	является константой типа int, а не word. Явно указать на второй тип 
	можно, написав 1u вместо 1.
*******************************************************************************
*/

template<size_t _n> class Word
{
// длина
public:
	//! раскрытие длины _n
	enum {n = _n};

// поддержка ссылок на отдельные (!) биты слов
public:
	class Reference
	{
		friend class Word;
	private:
		Word* _pWord;
		size_t _pos;

	public:
		Reference& operator=(bool bVal)
		{
			_pWord->Set(_pos, bVal);
			return *this;
		}

		Reference& operator=(const Reference& ref)
		{
			_pWord->Set(_pos, bool(ref));
			return *this;
		}

		operator bool() const
		{
			return _pWord->Test(_pos);
		}

	private:
		Reference(Word& w, size_t pos)
		{
			assert(pos < _n);
			_pWord = &w;
			_pos = pos;
		}
	};

// данные
protected:
	enum 
	{	// число битов в машинном слове
		_bitsperword = 8 * sizeof(word),
		// число машинных слов для хранения данных
		_wcount = (_n + _bitsperword - 1) / _bitsperword,
		// число неиспользуемых битов в последнем машинном слове
		_trimbits = _n % _bitsperword ? _bitsperword - _n % _bitsperword : 0,
	};
	// машинные слова
	word _words[_wcount];

	//! Очистка битов дополнения
	/*! Очищаются неиспользуемые биты в последнем слове представления. */
	void Trim()
	{	
		if (_trimbits) (_words[_wcount - 1] <<= _trimbits) >>= _trimbits;
	}

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
		if (val) _words[pos / _bitsperword] |= word(1) << pos % _bitsperword;
		else _words[pos / _bitsperword] &= ~(word(1) << pos % _bitsperword);
	}

	//! Установка символов
	/*! Символы с номерами pos1 <= pos < pos2 заполняются значением val. */
	void Set(size_t pos1, size_t pos2, bool val)
	{	
		// номера слов представлений
		size_t wpos1 = pos1 / _bitsperword, wpos2 = pos2 / _bitsperword, pos;
		// в одном слове?
		if (wpos1 == wpos2)
		{
			for (pos = pos1; pos < pos2; Set(pos++, val));
			return;
		}
		// начинаем посередине слова представления?
		if (pos = pos1 % _bitsperword)
		{
			(_words[wpos1] <<= (_bitsperword - pos)) >>= _bitsperword - pos;
			if (val) _words[wpos1] |= word(-1) << pos;
			wpos1++;
		}
		// заканчиваем посередине слова представления?
		if (pos = pos2 % _bitsperword)
		{
			(_words[wpos2] >>= pos) <<= pos;
			if (val) _words[wpos2] |= word(-1) >> (_bitsperword - pos);
		}
		// заполняем полные слова
		for (pos = wpos1; pos < wpos2; _words[pos++] = (val ? -1 : 0));
	}

	//! Заполнение константой
	/*! Слово заполняется символом val. */
	void SetAll(bool val)
	{	
		for (size_t pos = 0; pos < _wcount; pos++)
			_words[pos] = val ? -1 : 0;
		if (val) Trim();
	}

	//! Заполнение нулями
	/*! Слово обнуляется. */
	void SetAllZero()
	{	
		for (size_t pos = 0; pos < _wcount; pos++)
			_words[pos] = 0;
	}

	//! Значение символа
	/*! Возвращается значение символа с номером pos. */
	bool Test(size_t pos) const
	{
		assert(pos < _n);
		return (_words[pos / _bitsperword] & 
			(word(1) << pos % _bitsperword)) != 0;
	}

	//! Значение символа
	/*! Возвращается значение символа с номером pos. */
	bool operator[](size_t pos) const
	{
		return Test(pos);
	}

	//! Значение символа
	/*! Возвращается ссылка на символ с номером pos.
        Ссылку можно использовать в обеих частях оператора присваивания. 
        \par Пример:
		\code	 
        Word<10> w;\n w[1] = 0; w[2] = w[1];...
		\endcode */
	Reference operator[](size_t pos)
	{
		return Reference(*this, pos);
	}

	//! Инверсия символа
	/*! Инвертировать символ с номером pos. */
	Word& Flip(size_t pos)
	{	
		assert(pos < _n);
		_words[pos / _bitsperword] ^= word(1) << pos % _bitsperword;
		return *this;
	}

	//! Инверсия слова
	/*! Инвертировать все символы слова. */
	Word& FlipAll()
	{	
		for (size_t pos = 0; pos < _wcount; _words[pos++] ^= -1);
		Trim();
		return *this;
	}

	//! Слово из одинаковых символов?
	/*! Проверяется, что слово состоит из символов val. */
	bool IsAll(bool val) const
	{
		// пустое слово?
		if (_wcount == 0)
			return false;
		// проверить все слова представления, кроме последнего
		size_t pos = 0;
        for (; pos + 1 < _wcount; pos++)
			if (_words[pos] != (val ? -1 : 0))
				return false;
		// проверить последнее слово представления
		return _words[pos] == (val ? word(-1) << _trimbits >> _trimbits : 0);
	}

	//! Нулевое слово?
	/*! Проверяется, что все символы слова нулевые. */
	bool IsAllZero() const
	{
		for (size_t pos = 0; pos < _wcount; pos++)
			if (_words[pos] != 0)
				return false;
		return true;
	}

	//! Обратный порядок символов
	/*! Символы слова перезаписываются в обратном порядке. */
	Word& Reverse()
	{	
		for (size_t start = 0, end = _n; start + 1 < end; start++, end--)
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

	//! Устанавка слова представления
	/*! Устанавливается значение val слова представления 
		с номером pos.*/
	void SetWord(size_t pos, word val)
	{	
		assert(pos < _wcount);
		_words[pos] = val;
		if (pos == _wcount - 1) Trim();
	}

	//! Обмен
	/*! Производится обмен символами со словом wRight. */
	void Swap(Word& wRight)
	{
		for (size_t pos = 0; pos < _wcount; pos++)
		{
			word w = wRight.GetWord(pos);
			wRight.SetWord(pos, _words[pos]);
			_words[pos] = w;
		}
	}

	//! Вес
	/*! Определяется вес (число ненулевых символов) слова. */
	size_t Weight() const
	{	
		static char _bitsperbyte[256] = 
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
		for (size_t pos = 0; pos < _wcount * sizeof(word); pos++)
				weight += _bitsperbyte[((u8*)_words)[pos]];
		return weight;
	}

	//! Бит четности
	/*! Определяется бит четности (сумма символов mod 2). */
	bool Parity() const
	{	
		return bool(Weight() & 1);
	}

// манипуляции с наборами символов
public:
	//! Сдвиг влево
	/*! Сдвиг символов слова влево с записью нулей в правую часть:
		\code
		w[0]w[1]...w[n-1] -> w[shift]w[shift + 1]...w[n-1]00...0.
		\endcode */
	Word& Shl(size_t shift)
	{	
		if (shift < _n)
		{
			size_t wshift = shift / _bitsperword, pos;
			// величина сдвига не кратна длине слова?
			if (shift %= _bitsperword)
			{
				// сдвиг всех слов, кроме последнего
				for (pos = 0; pos + wshift + 1 < _wcount; pos++)
					_words[pos] = (_words[pos + wshift] >> shift) |
						(_words[pos + wshift + 1] << (_bitsperword - shift));
				// последнее слово
				_words[pos++] = _words[pos + wshift] >> shift;
			}
			// величина сдвига кратна длине слова
			else for (pos = 0; pos + wshift < _wcount; pos++)
				_words[pos] = _words[pos + wshift];
			// обнуление правых слов
			for (; pos < _wcount; _words[pos++] = 0);
		}
		else SetAllZero();
		return *this;
	}

	//! Сдвиг вправо
	/*! Сдвиг символов слова вправо с записью нулей в левую часть:
		\code
		w[0]w[1]...w[n-1] -> 00...0w[0]w[1]...w[n-1-shift].
		\endcode */
	Word& Shr(size_t shift)
	{	
		if (shift < _n)
		{
			size_t wshift = shift / _bitsperword, pos;
			// величина сдвига не кратна длине слова?
			if (shift %= _bitsperword)
			{
				// сдвиг всех слов, кроме первого
				for (pos = _wcount - 1; pos > wshift; pos--)
					_words[pos] = (_words[pos - wshift] << shift) |
						(_words[pos - wshift - 1] >> (_bitsperword - shift));
				// первое слово
				_words[pos--] = _words[pos - wshift] << shift;
			}
			// величина сдвига кратна длине слова
			else for (pos = _wcount - 1; pos + 1 > wshift; pos--)
					_words[pos] = _words[pos - wshift];
			// очистка последнего слова
			Trim();
			// обнуление левых слов
			for (; pos != -1; _words[pos--] = 0);
		}
		else SetAllZero();
		return *this;
	}

	//! Циклический сдвиг влево
	/*! Циклический сдвиг символов слова влево. 
		\pre _n != 0. */
	Word& Rotl(size_t shift)
	{	
		assert(_n != 0);
		shift %= _n;
		Word w(*this);
		return Shl(shift) |= w.Shr(_n - shift);
	}

	//! Циклический сдвиг вправо
	/*! Циклический сдвиг символов слова вправо.
		\pre _n != 0. */
	Word& Rotr(size_t shift)
	{	
		assert(_n != 0);
		shift %= _n;
		Word w(*this);
		return Shl(_n - shift) |= w.Shr(shift);
	}

	//! Выбор левой части
	/*! Левые (первые) _m символов слова записываются в w. 
		\pre this != &w.
		\return Ссылка на w.	*/
	template<size_t _m> 
	Word<_m>& GetLeft(Word<_m>& w) const
	{
		assert(_m <= _n && this != (void*)&w);
		for (size_t pos = 0; pos < w.WordSize(); pos++)
			w.SetWord(pos, _words[pos]);
		return w;
	}

	//! Выбор левой части
	/*! Возвращаются левые (первые) _m символов слова. */
	template<size_t _m>
	Word<_m> GetLeft() const
	{
		return GetLeft(Word<_m>());
	}

	//! Установка левой части
	/*! Левые (первые) _m символов слова устанавливаются по w. 
		\pre this != &w.
		\return Ссылка на само слово. */
	template<size_t _m>
	Word& SetLeft(const Word<_m>& w)
	{
		assert(_m <= _n && this != (void*)&w);
		size_t pos = 0;
		for (; pos + 1 < w.WordSize(); _words[pos++] = w.GetWord(pos));
		// неполное последнее слово mon?
		if (size_t trim = _m % _bitsperword)
		{
			// очищаем младшие trim битов
			(_words[pos] >>= trim) <<= trim;
			// устанавливаем младшие trim битов
			_words[pos] |= w.GetWord(pos);
		}
		else _words[pos] = w.GetWord(pos);
		return *this;
	}

	//! Выбор правой части
	/*! Правые (последние) _m символов слова записываются в w.
		\pre this != &w.
		\return Ссылка на w.	*/
	template<size_t _m>
	Word<_m>& GetRight(Word<_m>& w) const
	{
		assert(_m <= _n && this != (void*)&w);
		// первое слово, в котором начинается правая часть
		size_t start = (_n - _m) / _bitsperword;
		// правая часть начинается посередине слова (со смещением offset)?
		size_t pos;
		if (size_t offset = (_n - _m) % _bitsperword)
		{
			// объединять биты двух последовательных слов
			for (pos = 0; pos + start + 1 < _wcount; pos++)
				w.SetWord(pos, (_words[pos + start] >> offset) | 
					(_words[pos + start + 1] << (_bitsperword - offset)));
			// использовать биты последнего слова
			if (pos < w.WordSize())
				w.SetWord(pos, _words[pos + start] >> offset);
		}
		else
			for (pos = 0; pos < w.WordSize(); pos++)
				w.SetWord(pos, _words[pos + start]);
		return w;
	}

	//! Выбор правой части
	/*! Возвращаются правые (последние) _m символов слова. */
	template<size_t _m>
	Word<_m> GetRight() const
	{
		return GetRight(Word<_m>());
	}

	//! Установка правой части
	/*! Правые (последние) _m символов слова устанавливаются по w.
		\pre this != &w.
		\return ссылка на само слово. */
	template<size_t _m>
	Word& SetRight(const Word<_m>& w)
	{
		assert(_m <= _n && this != (void*)&w);
		// первое слово, в котором начинается правая часть
		size_t start = (_n - _m) / _bitsperword;
		// правая часть начинается посередине слова (со смещением offset)?
		if (size_t offset = (_n - _m) % _bitsperword)
		{
			// очистить старшие _bitsperword - offset битов _words[start]
			(_words[start] <<= _bitsperword - offset) >>= 
				_bitsperword - offset;
			// и записать в них младшие биты первого слова w
			_words[start] |= w.GetWord(0) << offset;
			// далее объединять биты двух последовательных слов w
			size_t pos;
			for (pos = 1; pos < w.WordSize(); pos++)
				_words[pos + start] = (w.GetWord(pos) << offset) |
					(w.GetWord(pos - 1) >> (_bitsperword - offset));
			// обработать остатки
			if (pos + start < _wcount)
				_words[pos + start] = 
					w.GetWord(pos - 1) >> (_bitsperword - offset);
		}
		else for (size_t pos = 0; pos < w.WordSize(); pos++)
				_words[pos + start] = w.GetWord(pos);
		// подчистить
		Trim();
		return *this;
	}


	//! Упаковка
	/*! В слове *this удаляются (со сдвигом влево) символы с индексами pos 
		такими,	что wMask[pos] == 0. */
	Word& Pack(const Word& wMask)
	{
		size_t pos = 0, posMask = 0;
		for (posMask = 0; posMask < _n; posMask++)
			if (wMask.Test(posMask))
				Set(pos++, Test(posMask));
		// добиваем нулями
		for (; pos < _n && pos % _bitsperword; Set(pos++, 0));
		for ((pos += _bitsperword - 1) /= _bitsperword; pos < _wcount; 
			_words[pos++] = 0);
		return *this;
	}

	//! Распаковка
	/*! В слово *this вставляются нулевые символы с индексами pos такими,
		что wMask[pos] == 0. */
	Word& Unpack(const Word& wMask)
	{
		size_t pos = wMask.Weight(), posMask = _n;
		while (posMask--)
			Set(posMask, wMask.Test(posMask) ? Test(--pos) : 0);
		return *this;
	}

	//! Перестановка
	/*! Слово w[0]w[1]...w[n-1]	заменяется на слово u[0]u[1]...u[n-1],
		в котором u[i] == w[pi[i]], если pi[i] != -1 и u[i] == 0 
		в противном случае. */
	Word& Permute(const size_t pi[_n])
	{	
		Word<_n> temp;
		for (size_t pos = 0; pos < _n; ++pos)
			temp.Set(pos, pi[pos] == - 1 ? 0 : Test(pi[pos]));
		return operator=(temp);
	}

// лексикографический порядок
public:
	//! Сравнение
	/*! Выполняется лексикографическое сравнение со словом wRight. 
		\return -1 (<), 0 (=), 1 (>). */
	int Compare(const Word& wRight) const
	{
		for (size_t pos = _wcount - 1; pos != -1; pos--)
			if (_words[pos] > wRight.GetWord(pos)) return 1;
			else if (_words[pos] < wRight.GetWord(pos)) return -1;
		return 0;
	}

	//! Сравнение
	/*! Выполняется лексикографическое сравнение со словом wRight 
		другой длины. 
		\return -1 (<), 0 (=), 1 (>). */
	template<size_t _m>
	int Compare(const Word<_m>& wRight) const
	{
		size_t pos;
		if (_n > _m) 
		{
			for (pos = _wcount - 1; pos != wRight.WordSize() - 1; pos--)
				if (GetWord(pos) != 0) return 1;
		}
		else for (pos = wRight.WordSize() - 1; pos != _wcount - 1; pos--)
		if (wRight.GetWord(pos) != 0) return -1;
		for (; pos != -1; pos--)
			if (_words[pos] > wRight.GetWord(pos)) return 1;
			else if (_words[pos] < wRight.GetWord(pos)) return -1;
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
			while (pos < _wcount && ++_words[pos] == 0) pos++;
			// перенос не попал в последнее слово представления?
			if (pos + 1 < _wcount) return true;
			// прошли последнее слово?
			if (pos == _wcount) return false;
			// остановились в последнем слове
			Trim(); 
			return _words[pos] != 0;
		}
		// ищем начало серии из единиц
		while (pos < _n && !Test(pos)) pos++;
		// единиц нет?
		if (pos == _n) return false;
		// ищем окончание серии
		size_t end = pos + 1;
		while (end < _n && Test(end)) end++;
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
			while (pos < _wcount && --_words[pos] == -1) pos++;
			// перенос не попал или остался в последнем слове представления?
			if (pos < _wcount) return true;
			Trim();
			return false;
		}
		// ищем первую единицу
		while (pos < _n && !Test(pos)) pos++;
		// единиц нет?
		if (pos == _n) return false;
		// начинается с нулей?
		if (pos > 0)
		{
			// передвигаем единицу влево
			Set(pos - 1, 1); Set(pos, 0);
			return true;
		}
		// ищем окончание серии из единиц
		size_t end = 1;
		while (end < _n && Test(end)) end++;
		// ищем следующую единицу
		pos = end;
		while (pos < _n && !Test(pos)) pos++;
		// серия слева: 1^end 00...0?
		if (pos == _n)
		{
			// возвращаемся к последнему слову с данным весом
			Last(end);
			return false;
		}
		// сдвигаем 1^{end}0^{pos-end}1... -> 0^{pos-end-1}1^{end+1}0...
		Set(0, pos - end - 1, 0); Set(pos - end - 1, pos, 1); Set(pos, 0);
		return true;
	}

	//! Задать наудачу
	/*! Генерация случайного слова. */
	Word& Rand()
	{
		Env::RandMem(_words, sizeof(_words));
		Trim();
		return *this;
	}

// операции
public:
	//! Присваивание
	/*! Присваивание слову значения-слова wRight. */
	Word& operator=(const Word& wRight)
	{
		if (&wRight != this)
			for (size_t pos = 0; pos < _wcount; pos++)
				_words[pos] = wRight.GetWord(pos);
		return *this;
	}

	//! Присваивание
	/*! Присваивание слову значения-слова wRight другой размерности. */
	template<size_t _m>
	Word& operator=(const Word<_m>& wRight)
	{
		size_t pos;
		for (pos = 0; pos < (_n < _m ? _wcount : wRight.WordSize()); pos++)
			_words[pos] = wRight.GetWord(pos);
		if (_n < _m) 
			Trim();
		else for (; pos < _wcount; _words[pos++] = 0);
		return *this;
	}

	//! Присваивание
	/*! Присваивание слову значения-машинного слова wRight. */
	Word& operator=(word wRight)
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
	bool operator==(const Word<_m>& wRight) const
	{	
		return Compare(wRight) == 0;
	}

	//! Равенство
	/*! Проверяется равенство машинному слову wRight. */
	bool operator==(word wRight) const
	{	
		if (_words[0] != wRight)
			return false;
		for (size_t pos = 1; pos < _wcount; pos++)
			if (_words[pos] != 0)
				return false;
		return true;
	}

	//! Неравенство
	/*! Проверяется неравенство слову wRight. */
	template<size_t _m>
	bool operator!=(const Word<_m>& wRight) const
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
	bool operator<(const Word<_m>& wRight) const
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
	bool operator<=(const Word<_m>& wRight) const
	{	
		return Compare(wRight) <= 0;
	}

	//! Не больше?
	/*! Проверяется, что слово не больше машинного слова wRight. */
	bool operator<=(word wRight) const
	{	
		if (_words[0] > wRight)
			return false;
		for (size_t pos = 1; pos < _wcount; pos++)
			if (_words[pos] > 0)
				return false;
		return true;
	}

	//! Больше?
	/*! Проверяется, что слово лексикографические больше wRight. */
	template<size_t _m>
	bool operator>(const Word<_m>& wRight) const
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
	bool operator>=(const Word<_m>& wRight) const
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
	Word operator~() const
	{	
		return Word(*this).FlipAll();
	}

	//! AND
	/*! Выполняется логическое умножение символов на 
		соответствующие символы слова wRight. */
	Word& operator&=(const Word& wRight)
	{	
		for (size_t pos = 0; pos < _wcount; pos++)
			_words[pos] &= wRight.GetWord(pos);
		return *this;
	}

	//! AND
	/*! Выполняется логическое умножение символов на 
		соответствующие символы слова wRight другой длины. */
	template<size_t _m>
	Word& operator&=(const Word<_m>& wRight)
	{	
		for (size_t pos = 0; pos < (_n < _m ? _wcount : wRight.WordSize());)
			_words[pos++] &= wRight.GetWord(pos);
		return *this;
	}

	//! AND
	/*! Выполняется логическое умножение символов на 
		соответствующие символы машинного слова wRight. */
	Word& operator&=(word wRight)
	{	
		_words[0] &= wRight;
		Trim();
		return *this;
	}

	//! OR
	/*! Выполняется логическое сложение символов с 
		соответствующими символами слова wRight. */
	Word& operator|=(const Word& wRight)
	{	
		for (size_t pos = 0; pos < _wcount; pos++)
			_words[pos] |= wRight.GetWord(pos);
		return *this;
	}

	//! OR
	/*! Выполняется логическое сложение символов с
		соответствующими символами слова wRight другой длины. */
	template<size_t _m>
	Word& operator|=(const Word<_m>& wRight)
	{	
		for (size_t pos = 0; pos < (_n < _m ? _wcount : wRight.WordSize());)
			_words[pos++] |= wRight.GetWord(pos);
		Trim();
		return *this;
	}

	//! OR
	/*! Выполняется логическое сложение символов с 
		соответствующими символами машинного слова wRight. */
	Word& operator|=(word wRight)
	{	
		_words[0] &= wRight;
		Trim();
		return *this;
	}

	//! XOR
	/*! Выполняется исключающее логическое сложение символов с 
		соответствующими символами слова wRight. */
	Word& operator^=(const Word& wRight)
	{	
		for (size_t pos = 0; pos < _wcount; pos++)
			_words[pos] ^= wRight.GetWord(pos);
		return *this;
	}

	//! XOR
	/*! Выполняется исключающее логическое сложение символов с
		соответствующими символами слова wRight другой длины. */
	template<size_t _m>
	Word& operator^=(const Word<_m>& wRight)
	{	
		for (size_t pos = 0; pos < (_n < _m ? _wcount : wRight.WordSize());)
			_words[pos++] ^= wRight.GetWord(pos);
		Trim();
		return *this;
	}

	//! XOR
	/*! Выполняется исключающее логическое сложение символов с 
		соответствующими символами машинного слова wRight. */
	Word& operator^=(word wRight)
	{	
		_words[0] ^= wRight;
		Trim();
		return *this;
	}

	//! Сдвиг влево
	/*! Выполняется сдвиг символов слова влево. */
	Word& operator<<=(size_t shift)
	{	
		return Shl(shift);
	}

	//! Сдвиг влево
	/*! Возвращается слово со сдвинутыми влево символами. */
	Word operator<<(size_t shift) const
	{	
		return Word(*this).Shl(shift);
	}

	//! Сдвиг вправо
	/*! Выполняется сдвиг символов слова вправо. */
	Word& operator>>=(size_t shift)
	{	
		return Shr(shift);
	}

	//! Сдвиг вправо
	/*! Возвращается слово со сдвинутыми вправо символами. */
	Word operator>>(size_t shift) const
	{	
		return Word(*this).Shr(shift);
	}


// конструкторы
public:
	//! Конструктор по умолчанию
	/*! Создается нулевое слово. */
	Word()
	{
		SetAllZero();
	}
	
	//! Конструктор по машинному слову
	/*! Создается копия машинного слова wRight 
		(возможно, с потерей старших битов wRight или, наоборот, 
		с добавлением нулевых символов). */
	Word(word wRight)
	{
		_words[0] = wRight;
		for (size_t pos = 1; pos < _wcount; _words[pos++] = 0);
		Trim();
	}

	//! Конструктор копирования
	/*! Создается копия слова wRight. */
	Word(const Word& wRight)
	{	
		for (size_t pos = 0; pos < _wcount; pos++)
			_words[pos] = wRight.GetWord(pos);
	}

	//! Конструктор копирования
	/*! Создается копия слова wRight другой длины. */
	template<size_t _m> 
	Word(const Word<_m>& wRight)
	{
		size_t pos;
		for (pos = 0; pos < (_n < _m ? _wcount : wRight.WordSize()); pos++)
			_words[pos] = wRight.GetWord(pos);
		if (_n < _m)
			Trim();
		else 
			for (; pos < _wcount; _words[pos++] = 0);
	}
};

//! Равенство
/*! Проверяется равенство машинного слова wLeft и слова wRight. */
template<size_t _n> inline bool
operator==(word wLeft, const Word<_n>& wRight)
{
	return wRight == wLeft;
}

//! Неравенство
/*! Проверяется неравенство машинного слова wLeft и слова wRight. */
template<size_t _n> inline bool
operator!=(word wLeft, const Word<_n>& wRight)
{
	return wRight != wLeft;
}

//! Меньше?
/*! Проверяется, что машинное слово wLeft меньше слова wRight. */
template<size_t _n> inline bool
operator<(word wLeft, const Word<_n>& wRight)
{
	return wRight > wLeft;
}

//! Не больше?
/*! Проверяется, что машинное слово wLeft не больше слова wRight. */
template<size_t _n> inline bool
operator<=(word wLeft, const Word<_n>& wRight)
{
	return wRight >= wLeft;
}

//! Больше?
/*! Проверяется, что машинное слово wLeft больше слова wRight. */
template<size_t _n> inline bool
operator>(word wLeft, const Word<_n>& wRight)
{
	return wRight < wLeft;
}

//! Не меньше?
/*! Проверяется, что машинное слово wLeft не меньше слова wRight. */
template<size_t _n> inline bool
operator>=(word wLeft, const Word<_n>& wRight)
{
	return wRight <= wLeft;
}

//! AND
/*! Определяется слово wLeft & wRight. */
template<size_t _n, size_t _m> inline Word<MAX2(_n, _m)> 
operator&(const Word<_n>& wLeft, const Word<_m>& wRight)
{	
	return Word<MAX2(_n,_m)>(wLeft) &= wRight;
}

//! AND
/*! Определяется слово wLeft & wRight (wRight -- машинное слово). */
template<size_t _n> inline Word<MAX2(_n, sizeof(word) * 8)> 
operator&(const Word<_n>& wLeft, word wRight)
{	
	return Word<MAX2(_n, sizeof(word) * 8)>(wLeft) &= wRight;
}

//! AND
/*! Определяется слово wLeft & wRight (wLeft -- машинное слово). */
template<size_t _n> inline Word<MAX2(_n, sizeof(word) * 8)> 
operator&(word wLeft, const Word<_n>& wRight)
{	
	return Word<MAX2(_n, sizeof(word) * 8)>(wRight) &= wLeft;
}

//! OR
/*! Определяется слово wLeft | wRight. */
template<size_t _n, size_t _m> inline Word<MAX2(_n, _m)> 
operator|(const Word<_n>& wLeft, const Word<_m>& wRight)
{	
	return Word<MAX2(_n, _m)>(wLeft) |= wRight;
}

//! OR
/*! Определяется слово wLeft | wRight (wRight -- машинное слово). */
template<size_t _n> inline Word<MAX2(_n, sizeof(word) * 8)> 
operator|(const Word<_n>& wLeft, word wRight)
{	
	return Word<MAX2(_n, sizeof(word) * 8)>(wLeft) |= wRight;
}

//! OR
/*! Определяется слово wLeft | wRight (wLeft -- машинное слово). */
template<size_t _n> inline Word<MAX2(_n, sizeof(word) * 8)> 
operator|(word wLeft, const Word<_n>& wRight)
{	
	return Word<MAX2(_n, sizeof(word) * 8)>(wRight) |= wLeft;
}

//! XOR
/*! Определяется слово wLeft ^ wRight. */
template<size_t _n, size_t _m> inline Word<MAX2(_n, _m)> 
operator^(const Word<_n>& wLeft, const Word<_m>& wRight)
{	
	return Word<MAX2(_n, _m)>(wLeft) ^= wRight;
}

//! XOR
/*! Определяется слово wLeft ^ wRight (wRight -- машинное слово). */
template<size_t _n> inline Word<MAX2(_n, sizeof(word) * 8)> 
operator^(const Word<_n>& wLeft, word wRight)
{	
	return Word<MAX2(_n, sizeof(word) * 8)>(wLeft) ^= wRight;
}

//! XOR
/*! Определяется слово wLeft ^ wRight (wLeft -- машинное слово). */
template<size_t _n> inline Word<MAX2(_n, sizeof(word) * 8)> 
operator^(word wLeft, const Word<_n>& wRight)
{	
	return Word<MAX2(_n, sizeof(word) * 8)>(wRight) ^= wLeft;
}

//! Конкатенация слов
/*! Слова wLeft и wRight конкатенируются. */
template<size_t _n1, size_t _n2> inline Word<_n1 + _n2> 
Concatenate(const Word<_n1>& wLeft, const Word<_n2>& wRight)
{	
	return Word<_n1 + _n2>().SetLeft(wLeft).SetRight(wRight);
}

//! Конкатенация слов
/*! Слова wLeft и wRight конкатенируются. 
	\remark Запись a || b соответствует устоявшимся
	математическим обозначениям. */
template<size_t _n1, size_t _n2> inline Word<_n1 + _n2> 
operator||(const Word<_n1>& wLeft, const Word<_n2>& wRight)
{	
	return Word<_n1 + _n2>().SetLeft(wLeft).SetRight(wRight);
}

//! Вывод в поток
/*! Слово wRight выводится в поток os. */
template<class _Char, class _Traits, size_t _n> inline 
std::basic_ostream<_Char, _Traits>& 
operator<<(std::basic_ostream<_Char, _Traits>& os, const Word<_n>& wRight)
{
	for (size_t pos = 0; pos < wRight.Size(); pos++)
		os << (wRight.Test(pos) ? "1" : "0");
	return os;
}

//! Ввод из потока
/*! Слово wRight читается из потока is. 
	\par Перед чтением слова пропускаются пробелы и знаки табуляции.
	Чтение невозможно, если в состоянии потока установлен флаг ошибки.
	\par Чтение слова прекращается, если 
	1) прочитаны все символы,
	2) достигнут конец потока,
	3) встретился недопустимый символ (не 0 или 1). 
	Во втором случае в состоянии потока будет установлен флаг 
	ios_base::eofbit. 
	Если не прочитано ни одного символа непустого слова, 
	то в состоянии потока будет установлен флаг ios_base::failbit, 
	без снятия которого дальнейшее чтение из потока невозможно. 
	При чтении из потока недопустимого символа он будет возвращен в поток.
	По окончании ввода слово wRight будет содержать 
	прочитанные символы, дополненные нулями. */
template<class _Char, class _Traits, size_t _n> inline
std::basic_istream<_Char, _Traits>& 
operator>>(std::basic_istream<_Char, _Traits>& is, Word<_n>& wRight)
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
			typename _Traits::int_type c1 = is.rdbuf()->sbumpc();
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
	else if (_n > 0) 
			is.setstate(std::ios_base::failbit);
	return is;
}

} // namespace GF2

#endif // __GF2_WORD