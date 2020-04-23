/*
*******************************************************************************
\file test.cpp
\brief Tests
\project GF2 [algebra over GF(2)]
\created 2016.07.06
\version 2020.04.23
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
Тесты 

#	wwTest: функционал WW;
#	orderTest: проверка тождественности OrderGrlex<6> и OrderGr<OrderLex<6> >;
#	bentTest: проверка бентовости функции Майораны-МакФарланда;
#	sboxTest: проверка криптографических характеристик 4-битового S-блока 
	(by.gost28147.params.1);
#	beltTest: проверка криптографических характеристик 8-битового S-блока belt
	(СТБ 34.101.31);
#	bashTest: базис Гребнера идеала, описывающего S-блок Bash;
#	bash2Test: формульное расширение размерности S-блока Bash, 
	проверка биективности, обращение и проверка степеней;
#	commuteTest: коммутируемые обратимые двоичные матрицы порядка 2.
*******************************************************************************
*/

bool wwTest()
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

bool orderTest()
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

bool bentTest()
{
	typedef MM<12> X;
	MP<12> p;
	BFunc<12> bf;
	// многочлен
	p = X(0,6) + X(1, 7) + X(2, 8) + X(3, 9);
	p += X({4, 10}) + X({5, 5, 11, 11, 11});
	// функция
	bf.From(p);
	return bf.IsBent();
}

bool sboxTest()
{
	static const word s_table[16] = {2,6,3,14,12,15,7,5,11,13,8,9,10,0,4,1};
	VSubst<4> s(s_table);
	// характеристики
	return s.Nl() == 4 && 
		s.Deg() == 3 && s.DegSpan() == 3 &&
		s.Dc(0) == 4 && s.Dc(1) == 4 && s.Dc(2) == 4 && s.Dc(3) == 3;
}

bool beltTest(bool verbose = false)
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

bool bashTest()
{
	typedef MOGrevlex<6> O;
	typedef MM<6> X;
	// S-блок и его идеал
	word s_table[] = {1, 2, 3, 4, 6, 7, 5, 0};
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

bool bashTest2()
{
	const size_t n = 3;
	WW<3 * n> v;
	VSubst<3 * n> s;
	do
	{
		WW<n> x0 = v.GetLo<n>();
		WW<n> x1 = v.GetLo<2 * n>().GetHi<n>();
		WW<n> x2 = v.GetHi<n>();
		WW<n> y0 = (x1 | ~x2) ^ x0.RotLo(1) ^ x1;
		WW<n> y1 = (x0 | x1) ^ x0 ^ x1 ^ x2;
		WW<n> y2 = (x0 & x1) ^ x1 ^ x2;
		s[x0 || x1 || x2] = y0 || y1 || y2;
	}
	while (v.Next());
	if (!s.IsBijection())
		return false;
	return (s.Deg() == 2 && s.Inverse().Deg() == 2);
}

bool commuteTest()
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
	return i.QuotientBasisDim() == (word)18;
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
	ret |= !Env::RunTest("wwTest", wwTest);
	ret |= !Env::RunTest("orderTest", orderTest);
	ret |= !Env::RunTest("bentTest", bentTest);
	ret |= !Env::RunTest("sboxTest", sboxTest);
	ret |= !Env::RunTest("beltTest", beltTest, true);
	ret |= !Env::RunTest("bashTest", bashTest);
	ret |= !Env::RunTest("bashTest2", bashTest2);
	ret |= !Env::RunTest("commuteTest", commuteTest);
	return ret;
}
