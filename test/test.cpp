/*
*******************************************************************************
\file test.cpp
\brief Tests
\project GF2 [GF(2) algebra library]
\author (С) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2016.07.06
\version 2016.07.22
\license This program is released under the MIT License. See Copyright Notices 
in GF2/info.h.
*******************************************************************************
*/

#include "gf2/buchb.h"
#include "gf2/func.h"
#include "gf2/ideal.h"
#include <sstream>

using namespace GF2;
using namespace std;

/*
*******************************************************************************
Явное проверочное инстанциирование
*******************************************************************************
*/

template class Word<127>;
	template class Monom<129>;
	template class ZZ<130>;

template struct OrderGr<OrderLR<OrderLex<65>, OrderGrlex<66> > >;
template struct OrderRL<OrderRev<OrderGrevlex<68> >, OrderLex<67> >;

template class MPoly<135, OrderLex<135> >;
template class Ideal<136, OrderGrevlex<136> >;
template class Buchb<137, OrderGrlex<137> >;

template class Func<5, int>;
	template class BFunc<6>;
	template class VFunc<7, 8>;
		template class VSubst<8>;
/*
*******************************************************************************
Тесты 

#	orderTest: проверка тождественности OrderGrlex<6> и OrderGr<OrderLex<6> >;
#	bentTest: проверка бентовости функции Майораны-МакФарланда;
#	sboxTest: проверка криптографических характеристик 4-битового S-блока 
	(by.gost28147.params.1);
#	bashTest: базис Гребнера идеала, описывающего S-блок Bash;
#	commuteTest: коммутируемые обратимые двоичные матрицы порядка 2.
*******************************************************************************
*/

bool orderTest()
{
	OrderGrlex<6> o1;
	OrderGr<OrderLex<6> > o2;
	Monom<6> m1;
	do
	{
		Monom<6> m2;
		do
		{
			if (o1.Compare(m1, m2) != o2.Compare(m1, m2))
				return false;
		}
		while (m2.Next());
	}
	while (m1.Next());
	return true;
}

bool bentTest()
{
	typedef Monom<12> X;
	MPoly<12> p;
	BFunc<12> bf;
	// многочлен
	p = X(0,6) + X(1, 7) + X(2, 8) + X(3, 9);
	p += X(4, 10) + X(5, 11);
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

bool bashTest()
{
	typedef OrderGrevlex<6> O;
	typedef Monom<6> X;
	// S-блок и его идеал
	word s_table[] = {1, 2, 3, 4, 6, 7, 5, 0};
	VSubst<3> s(s_table);
	Ideal<6, O> i;
	// i <- {x_{k + 3} - s_k(x_0, s_1, x_2): k = 0, 1, 2}
	MPoly<3, O> p;
	BFunc<3> bf;
	s.GetCoord(0, bf);
	bf.To(p);
	i.Insert(MPoly<6, O>(p) + X(3));
	s.GetCoord(1, bf);
	bf.To(p);
	i.Insert(MPoly<6, O>(p) + X(4));
	s.GetCoord(2, bf);
	bf.To(p);
	i.Insert(MPoly<6, O>(p) + X(5));
	// базис Гребнера
	Buchb<6, O> bb;
	bb.Init();
	bb.Update(i);
	bb.Process();
	bb.Done(i);
	// завершение
	Env::Trace("");
	return i.Size() == 14;
}

bool commuteTest()
{
	typedef OrderGrevlex<8> O;
	// система
	stringstream ss;
	ss << 
		"{ x0 x3 + x1 x2 + 1,"				/* обратимость первой матрицы */
		"  x1 x6 + x2 x5,"					/* коммутируемость */
		"  x1 x7 + x3 x5 + x0 x5 + x1 x4,"	/* коммутируемость */
		"  x2 x7 + x3 x6 + x0 x6 + x2 x4,"	/* коммутируемость */
		"  x4 x7 + x5 x6 + 1}";				/* обратимость второй матрицы */
	Ideal<8, O> i;
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
	bool code;
	int ret = 0;
	Env::Print("gf2/test [gf2 version %s]\n", Env::Version());
	Env::Print("orderTest: %s\n", (code = orderTest()) ? "OK" : "Err"), 
		ret |= !code;
	Env::Print("bentTest: %s\n", (code = bentTest()) ? "OK" : "Err"), 
		ret |= !code;
	Env::Print("sboxTest: %s\n", (code = sboxTest()) ? "OK" : "Err"), 
		ret |= !code;
	Env::Print("bashTest: %s\n", (code = bashTest()) ? "OK" : "Err"), 
		ret |= !code;
	Env::Print("commuteTest: %s\n", (code = commuteTest()) ? "OK" : "Err"), 
		ret |= !code;
	return ret;
}
