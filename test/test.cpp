/*
*******************************************************************************
\file test.cpp
\brief Tests
\project GF2 [algebra over GF(2)]
\created 2016.07.06
\version 2020.07.15
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

#include "gf2/buchb.h"
#include "gf2/func.h"
#include "gf2/mi.h"
#include <sstream>

using namespace GF2;
using namespace std;

/*
*******************************************************************************
Явное проверочное инстанциирование

\remark DR275 (https://wg21.cmeerw.net/cwg/issue275), которому следует Clang, 
требует явно указывать namespace при инстанциировании.
*******************************************************************************
*/

template class GF2::WW<127>;
	template class GF2::MM<129>;
	template class GF2::ZZ<130>;

template struct GF2::MOGr<MOLR<MOLex<65>, MOGrlex<66>>>;
template struct GF2::MORL<MORev<MOGrevlex<68>>, MOLex<67>>;

template class GF2::MP<135, MOLex<135>>;
template class GF2::MI<136, MOGrevlex<136>>;
template class GF2::Buchb<137, MOGrlex<137>>;

template class GF2::Func<5, int>;
	template class GF2::BFunc<6>;
	template class GF2::VFunc<7, 8>;
		template class GF2::VSubst<8>;

/*
*******************************************************************************
Тест testWW

Проверка функционала класса WW
*******************************************************************************
*/

bool testWW()
{
	// 1
	WW<127> w1;
	w1[12] = 1, w1.Flip(12), w1.Flip(12);
	if (w1.Weight() != 1)
		return false;
	w1.Set(14, 23, 1);
	if (w1.Reverse().Weight() != 1 + 23 - 14)
		return false;
	w1.SetAll(1);
	if (!w1.IsAll(1) || w1.Weight() != w1.Size())
		return false;
	// 2
	WW<126> w2;
	w1.SetLo(w2);
	if (w1[126] != 1 || w1.Weight() != 1)
		return false;
	// 3
	w1.FlipAll();
	w2[37] = 1;
	w1.SetHi(w2);
	if (w1[0] != 1 || w1[38] != 1 || w1.Weight() != 2)
		return false;
	// 4
	w1.RotHi(126 - 38);
	w2 |= w1;
	if (w2.Weight() != 2)
		return false;
	// 5
	word c = 0x7F00;
	w1 ^= w2;
	if (w1.Weight() != 2 || !WW<126>(w2 & c).IsAllZero())
		return false;
	// 6
	w2.Next(), w2.Next(), w2.Prev();
	w1 &= ~(w2 ^ c);
	w1 ^= (w2 ^ c);
	if (w1.Weight() != w2.Weight() + WW<15>(c).Weight())
		return false;
	// 7
	WW<127 + 126> w3(w1 || w2);
	if (w3.Weight() != w1.Weight() + w2.Weight())
		return false;
	// 8
	w3.FlipAll();
	WW<127> w4 = w3.GetLo<127>();
	if (w1.FlipAll() != w4)
		return false;
	// 9
	std::stringstream ss;
	ss << w1.Rand(), ss >> w4;
	if (w1 != w4)
		return false;
	return true;
}

/*
*******************************************************************************
Тест testMP

Проверка функционала класса MP

todo
*******************************************************************************
*/

bool testMP()
{
	typedef MM<6> X;
	typedef MOGrlex<6> O1;
	typedef MOGrevlex<6> O2;
	// 1
	MP<6> p1 = (X(0) + X(1)) * (X{ 1, 2 } + X{ 2, 3, 4 });
	MP<6> p2 = (X(0) + X(1)) + (X{ 1, 2 } + X{ 2, 3, 4 });
	// 2
	MP<6, O1> p3(p1);
	p3 /= X(0) + X(1);
	if (p3 != X(2) + X(0, 2) + X{ 2, 3, 4 })
		return false;
	// 3
	p3 = p1 * p2;
	MP<6, O2> p4 = p2;
	p4 *= p1;
	if (p3 != p4)
		return false;
	return true;
}

/*
*******************************************************************************
Тест testОrder

Проверка тождественности OrderGrlex<6> и OrderGr<OrderLex<6>>
*******************************************************************************
*/

bool testOrder()
{
	MOGrlex<6> o1;
	MOGr<MOLex<6> > o2;
	MM<6> m1;
	do
	{
		MM<6> m2;
		do
			if (o1.Compare(m1, m2) != o2.Compare(m1, m2))
				return false;
		while (m2.Next());
	}
	while (m1.Next());
	return true;
}

/*
*******************************************************************************
Тест testBFunc

Проверка функционала класса BFunc
*******************************************************************************
*/

bool testBFunc()
{
	typedef MM<4> X;
	MP<4> p, p1;
	BFunc<4> bf;
	Func<4, int> zf;
	// многочлен
	p = X(0, 1) + X(2, 3) + X{ 0, 1, 2, 3 };
	// преобразования
	bf.From(p);
	bf.To(zf);
	bf.From(zf);
	bf.To(p1);
	if (p != p1)
		return false;
	return true;
}

/*
*******************************************************************************
Тест testBent

Проверка бентовости функции Майораны-МакФарланда.
*******************************************************************************
*/

bool testBent()
{
	typedef MM<12> X;
	MP<12> p;
	BFunc<12> bf;
	// многочлен
	p = X(0, 6) + X(1, 7) + X(2, 8) + X(3, 9) + X(4, 10) + X(5, 11);
	// функция
	bf.From(p);
	return bf.IsBent();
}

/*
*******************************************************************************
Тест testBent2

Проверка бентовости функций из статьи [Rothaus O. On "bent" functions].
*******************************************************************************
*/

bool testBent2()
{
	typedef MM<6> X;
	BFunc<6> bf1, bf3, bf4;
	// функции -- представители классов 1, 3, 4
	bf1.From(X{ 0, 1, 2 } + 
		X{ 0, 3 } + X{ 1, 4 } + X{ 2, 5 });
	bf3.From(X{ 0, 1, 2 } + X{ 1, 3, 4 } + 
		X{ 0, 1 } + X{ 0, 3 } + X{ 1, 5 } +X{ 2, 4 } +X{ 3, 4 });
	bf4.From(X{ 0, 1, 2 } + X{ 1, 3, 4 } + X{ 2, 3, 5 } + 
		X{ 0, 3 } + X{ 1, 5 } + X{ 2, 3 } + X{ 2, 4 } + X{ 2, 5 } + 
		X{ 3, 4 } + X{ 3, 5 });
	// проверка
	return bf1.IsBent() && bf3.IsBent() && bf4.IsBent();
}

/*
*******************************************************************************
Тест testGOST

Проверка криптографических характеристик 4-битовых S-блоков ГОСТ 28147 
(заданы в СТБ 34.101.50, by.gost28147.params.1).
*******************************************************************************
*/

bool testGOST()
{
	static const word s_table[8][16] =
	{
		{2, 6, 3, 14, 12, 15, 7, 5, 11, 13, 8, 9, 10, 0, 4, 1}, 
		{8, 12, 9, 6, 10, 7, 13, 1, 3, 11, 14, 15, 2, 4, 0, 5}, 
		{1, 5, 4, 13, 3, 8, 0, 14, 12, 6, 7, 2, 9, 15, 11, 10}, 
		{4, 0, 5, 10, 2, 11, 1, 9, 15, 3, 6, 7, 14, 12, 8, 13}, 
		{7, 9, 6, 11, 15, 10, 8, 12, 4, 14, 1, 0, 5, 3, 13, 2}, 
		{14, 8, 15, 2, 6, 3, 9, 13, 5, 7, 0, 1, 4, 10, 12, 11}, 
		{9, 13, 8, 5, 11, 4, 12, 2, 0, 10, 15, 14, 1, 7, 3, 6}, 
		{11, 15, 10, 8, 1, 14, 3, 6, 9, 0, 4, 5, 13, 2, 7, 12}, 
	};
	for (size_t i = 0; i < 8; ++i)
	{
		VSubst<4> s(s_table[i]);
		// проверить инварианты add-семейства
		if (s.Dc(2) != 4 && s.Dc(3) != 4 - (i == 0))
			return false;
		// цикл по add-семейству
		for (word a = 0; a < 16; ++a)
		{
			if (s.Nl() != 4 ||
				s.Deg() != 3 && s.DegSpan() != 3 &&
				s.Dc(0) != 4 && s.Dc(1) != 4)
				return false;
			// к следующему представителю add-семейства
			word t = s[0], x;
			for (x = 0; x < 15; ++x)
				s[x] = s[x + 1];
			s[x] = t;
		}
	}
	return true;
}

/*
*******************************************************************************
Тест testBelt 

Проверка криптографических характеристик 8-битового S-блока Belt 
(СТБ 34.101.31).
*******************************************************************************
*/

bool testBelt(bool verbose = false)
{
	static const word h_table[256] = {
		0xB1,0x94,0xBA,0xC8,0x0A,0x08,0xF5,0x3B,0x36,0x6D,0x00,0x8E,0x58,0x4A,0x5D,0xE4,
		0x85,0x04,0xFA,0x9D,0x1B,0xB6,0xC7,0xAC,0x25,0x2E,0x72,0xC2,0x02,0xFD,0xCE,0x0D,
		0x5B,0xE3,0xD6,0x12,0x17,0xB9,0x61,0x81,0xFE,0x67,0x86,0xAD,0x71,0x6B,0x89,0x0B,
		0x5C,0xB0,0xC0,0xFF,0x33,0xC3,0x56,0xB8,0x35,0xC4,0x05,0xAE,0xD8,0xE0,0x7F,0x99,
		0xE1,0x2B,0xDC,0x1A,0xE2,0x82,0x57,0xEC,0x70,0x3F,0xCC,0xF0,0x95,0xEE,0x8D,0xF1,
		0xC1,0xAB,0x76,0x38,0x9F,0xE6,0x78,0xCA,0xF7,0xC6,0xF8,0x60,0xD5,0xBB,0x9C,0x4F,
		0xF3,0x3C,0x65,0x7B,0x63,0x7C,0x30,0x6A,0xDD,0x4E,0xA7,0x79,0x9E,0xB2,0x3D,0x31,
		0x3E,0x98,0xB5,0x6E,0x27,0xD3,0xBC,0xCF,0x59,0x1E,0x18,0x1F,0x4C,0x5A,0xB7,0x93,
		0xE9,0xDE,0xE7,0x2C,0x8F,0x0C,0x0F,0xA6,0x2D,0xDB,0x49,0xF4,0x6F,0x73,0x96,0x47,
		0x06,0x07,0x53,0x16,0xED,0x24,0x7A,0x37,0x39,0xCB,0xA3,0x83,0x03,0xA9,0x8B,0xF6,
		0x92,0xBD,0x9B,0x1C,0xE5,0xD1,0x41,0x01,0x54,0x45,0xFB,0xC9,0x5E,0x4D,0x0E,0xF2,
		0x68,0x20,0x80,0xAA,0x22,0x7D,0x64,0x2F,0x26,0x87,0xF9,0x34,0x90,0x40,0x55,0x11,
		0xBE,0x32,0x97,0x13,0x43,0xFC,0x9A,0x48,0xA0,0x2A,0x88,0x5F,0x19,0x4B,0x09,0xA1,
		0x7E,0xCD,0xA4,0xD0,0x15,0x44,0xAF,0x8C,0xA5,0x84,0x50,0xBF,0x66,0xD2,0xE8,0x8A,
		0xA2,0xD7,0x46,0x52,0x42,0xA8,0xDF,0xB3,0x69,0x74,0xC5,0x51,0xEB,0x23,0x29,0x21,
		0xD4,0xEF,0xD9,0xB4,0x3A,0x62,0x28,0x75,0x91,0x14,0x10,0xEA,0x77,0x6C,0xDA,0x1D,
	};
	VSubst<8> s(h_table);
	word a, x;
	// характеристики s
	if (s.Nl() != 102 || s.Deg() != 7 || s.DegSpan() != 6 ||
		s.Dc(0) != 8 || s.Dc(1) != 6 || s.Dc(2) != 3 || s.Dc(3) != 7)
		return false;
	if (!verbose)
		return true;
	// характеристики add-семейства
	for (a = 1; a < 256; ++a)
	{
		VSubst<8> t;
		for (x = 0; x < 256; ++x)
			t[x] = s[(x + a) % 256];
		if (t.Nl() < 96 || t.Deg() != 7 || t.Dc(0) > 10 || t.Dc(1) > 8)
			return false;
		Env::Trace("%d", (int)a);
	}
	Env::Trace("");
	return true;
}

/*
*******************************************************************************
Тест testBash 

Вычисление базиса Гребнера идеала, описывающего S-блок Bash (СТБ 34.101.77).
*******************************************************************************
*/

bool testBash()
{
	typedef MOGrevlex<6> O;
	typedef MM<6> X;
	// S-блок и его идеал
	const word s_table[] = {1, 2, 3, 4, 6, 7, 5, 0};
	VSubst<3> s(s_table);
	MI<6, O> i;
	// i <- {x_{k + 3} - s_k(x_0, s_1, x_2): k = 0, 1, 2}
	MP<3, O> p;
	BFunc<3> bf;
	s.GetCoord(0, bf);
	bf.To(p);
	i.Insert(MP<6, O>(p) + X(3));
	s.GetCoord(1, bf);
	bf.To(p);
	i.Insert(MP<6, O>(p) + X(4));
	s.GetCoord(2, bf);
	bf.To(p);
	i.Insert(MP<6, O>(p) + X(5));
	// базис Гребнера
	Buchb<6, O> bb;
	bb.Init();
	bb.Update(i);
	bb.Process();
	bb.Done(i);
	// завершение
	return i.Size() == 14 && i.QuotientBasisDim() == word(8);
}

/*
*******************************************************************************
Тест testBash2 

Формульное расширение размерности S-блока Bash, проверка биективности, 
обращение и проверка степеней.
*******************************************************************************
*/

bool testBash2()
{
	const size_t n = 3;
	WW<3 * n> v;
	VSubst<3 * n> s;
	do
	{
		auto x0 = v.GetLo<n>();
		auto x1 = v.GetLo<2 * n>().GetHi<n>();
		auto x2 = v.GetHi<n>();
		auto y0 = (x1 | ~x2) ^ x0.RotLo(1) ^ x1;
		auto y1 = (x0 | x1) ^ x0 ^ x1 ^ x2;
		auto y2 = (x0 & x1) ^ x1 ^ x2;
		s[x0 || x1 || x2] = y0 || y1 || y2;
	}
	while (v.Next());
	if (!s.IsBijection())
		return false;
	return (s.Deg() == 2 && s.Inverse().Deg() == 2);
}

/*
*******************************************************************************
Тест testCommute

Алгебраическое описание пар коммутируемых обратимым двоичных матриц порядка 2, 
контроль числа пар.
*******************************************************************************
*/

bool testCommute()
{
	typedef MOGrevlex<8> O;
	// система
	stringstream ss;
	ss << 
		"{ x0 x3 + x1 x2 + 1,"				/* обратимость первой матрицы */
		"  x1 x6 + x2 x5,"					/* коммутируемость */
		"  x1 x7 + x3 x5 + x0 x5 + x1 x4,"	/* коммутируемость */
		"  x2 x7 + x3 x6 + x0 x6 + x2 x4,"	/* коммутируемость */
		"  x4 x7 + x5 x6 + 1}";				/* обратимость второй матрицы */
	MI<8, O> i;
	ss >> i;
	// базис Гребнера
	Buchb<8, O> bb;
	bb.Init();
	bb.Update(i);
	bb.Process();
	bb.Done(i);
	// базис Гребнера?
	if (!i.IsGB())
		return false;
	// количество матриц
	return i.QuotientBasisDim() == word(18);
}

/*
*******************************************************************************
Тест testEM

Алгебраическая атака на 2-тактовую криптосистему EM (Even-Mansour):
	Ci = F(F(Pi ^ K1) ^ K2) ^ K3, i = 1, 2, ..., np.
Открытый текст Pi, шифртекст Сi и тактовые ключи K1, K2, K3 являются 3-битовыми
словами. 

Тактовая функция F является композицией S-блока Bash (см. testBash()) 
и циклического сдвига:
	F(a, b, c) = (a ^ b | ~c, b ^ a | c, c ^ a & b) >>> 1 =
		(c + a b, a + b c + c + 1, b + a c + a + c).
Таблица abc -> F(abc):
	000 -> 010
	100 -> 001
	010 -> 011
	110 -> 100
	001 -> 101
	101 -> 111
	011 -> 110
	111 -> 000

Раскладка переменных (Ti = S(Pi ^ K1)):
	K1 = x[0, 1, 2]
	K2 = x[3, 4, 5]
	K3 = x[6, 7, 8]
	Pi = x[0 + 9i, 1 + 9i, 2 + 9i]
	Ti = x[3 + 9i, 4 + 9i, 5 + 9i]
	Ci = x[6 + 9i, 7 + 9i, 8 + 9i]

Шифрматериал (K1 = 101, K2 = 110, K3 = 010):
	P1 = 000, T1 = F(101) = 111, C1 = F(T1 ^ K2) ^ K3 = S(001) ^ K3 = 111
	P2 = 001, T2 = F(100) = 001, C2 = F(T2 ^ K2) ^ K3 = S(111) ^ K3 = 010
	P3 = 100, T3 = F(001) = 101, C3 = F(T3 ^ K2) ^ K3 = S(011) ^ K3 = 100
	P4 = 101, T4 = F(000) = 010, C4 = F(T4 ^ K2) ^ K3 = S(100) ^ K3 = 011
*******************************************************************************
*/

bool testEM()
{
	const size_t np = 4;
	const size_t n = 9 + 9 * np;
	typedef MOGrlex<n> O;
	typedef MM<n> X;
	MI<n, O> s, t;
	Buchb<n, O> bb;
	// атака 
	for (size_t i = 1; i <= np; ++i)
	{
		// Ti = F(Pi ^ K1)
		s.Insert(X(3 + 9 * i) + X(2 + 9 * i) + X(2) +
			(X(0 + 9 * i) + X(0)) * (X(1 + 9 * i) + X(1)));
		s.Insert(X(4 + 9 * i) + X(0 + 9 * i) + X(0) +
			(X(1 + 9 * i) + X(1)) * (X(0 + 9 * i) + X(0)) +
			X(2 + 9 * i) + X(2) + true);
		s.Insert(X(5 + 9 * i) + X(1 + 9 * i) + X(1) +
			(X(0 + 9 * i) + X(0)) * (X(2 + 9 * i) + X(2)) +
			X(0 + 9 * i) + X(0) + X(2 + 9 * i) + X(2));
		// Ci ^ K3 = S(Ti ^ K2)
		s.Insert(X(6 + 9 * i) + X(6) + X(5 + 9 * i) + X(5) +
			(X(3 + 9 * i) + X(3)) * (X(4 + 9 * i) + X(4)));
		s.Insert(X(7 + 9 * i) + X(7) + X(3 + 9 * i) + X(3) +
			(X(4 + 9 * i) + X(4)) * (X(5 + 9 * i) + X(5)) +
			X(5 + 9 * i) + X(5) + true);
		s.Insert(X(8 + 9 * i) + X(8) + X(4 + 9 * i) + X(4) +
			(X(3 + 9 * i) + X(3)) * (X(5 + 9 * i) + X(5)) +
			X(3 + 9 * i) + X(3) + X(5 + 9 * i) + X(5));
		// ввод шифрматериала
		static const string material[5] =
		{
			"",
			"{x9, x10, x11, x15 + 1, x16 + 1, x17 + 1}",		// 000 -> 111
			"{x18, x19, x20 + 1, x24, x25 + 1, x26}",			// 001 -> 010
			"{x27 + 1, x28, x29, x33 + 1, x34, x35}",			// 100 -> 100
			"{x36 + 1, x37, x38 + 1, x42, x43 + 1, x44 + 1}",	// 101 -> 011
		};
		stringstream ss;
		ss << material[i];
		ss >> t;
		s.Insert(t);
		// базис Гребнера
		bb.Init();
		bb.Update(s);
		bb.Process();
		bb.Done(t);
		// проверка числа решений
		static const word nsol[5] = { 0, 64, 8, 2, 1 };
		if (t.QuotientBasisDim() != nsol[i])
			return false;
	}
	// проверка ключа
	WW<9> key;
	for (size_t i = 0; i < 9; ++i)
		if (t.IsContain(X(i) + true))
			key[i] = true;
	// key[8..0] =? 010 011 101
	return key == (word)0x009D;
}

/*
*******************************************************************************
main
*******************************************************************************
*/

int main()
{
	int ret = 0;
	Env::Print("gf2/test [gf2 version %s]\n", Env::Version());
	ret |= !Env::RunTest("testWW", testWW);
	ret |= !Env::RunTest("testMP", testMP);
	ret |= !Env::RunTest("testOder", testOrder);
	ret |= !Env::RunTest("testBFunc", testBFunc);
	ret |= !Env::RunTest("testBent", testBent);
	ret |= !Env::RunTest("testBent2", testBent2);
	ret |= !Env::RunTest("testGOST", testGOST);
	ret |= !Env::RunTest("testBelt", testBelt, true);
	ret |= !Env::RunTest("testBash", testBash);
	ret |= !Env::RunTest("testBash2", testBash2);
	ret |= !Env::RunTest("testCommute", testCommute);
	ret |= !Env::RunTest("testEM", testEM);
	return ret;
}
