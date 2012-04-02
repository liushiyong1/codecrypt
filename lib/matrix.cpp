
#include "codecrypt.h"

using namespace ccr;

void matrix::unit (uint size)
{
	clear();
	resize (size);
	for (uint i = 0; i < size; ++i) {
		item (i).resize (size, 0);
		item (i) [i] = 1;
	}
}

matrix matrix::operator* (const matrix&a)
{
	matrix r = *this;
	r.mult (a);
	return r;
}

void matrix::compute_transpose (matrix&r)
{
	uint h = height(), w = width(), i, j;
	r.resize (h);
	for (i = 0; i < h; ++i) {
		r[i].resize (w);
		for (j = 0; j < w; ++j) r[i][j] = item (j) [i];
	}
}

void matrix::mult (const matrix&right)
{
	//trivial multiply. TODO strassen algo for larger matrices.
	matrix leftT;
	compute_transpose (leftT);
	uint w = right.width(), h = leftT.width(), i, j;
	resize (w);
	for (i = 0; i < w; ++i) {
		item (i).resize (h);
		for (j = 0; j < h; ++j) item (i) [j] = leftT[j] * right[i];
	}
}

bool matrix::compute_inversion (matrix&res)
{
	//gauss-jordan elimination with inversion of the second matrix.
	//we are computing with transposed matrices for simpler row ops

	uint s = width();
	if (s != height() ) return false;
	matrix m, r;
	r.unit (s);
	this->compute_transpose (m);

	uint i, j;

	//gauss step, create a lower triangular out of m, mirror to r
	for (i = 0; i < s; ++i) {
		//we need pivoting 1 at [i][i]. If there's none, get it below.
		if (m[i][i] != 1) {
			for (j = i + 1; j < s; ++j) if (m[j][i] == 1) break;
			if (j == s) return false; //noninvertible
			m[i].add (m[j]);
			r[i].add (r[j]);
		}
		//remove 1's below
		for (j = i + 1; j < s; ++j) if (m[j][i]) {
				m[j].add (m[i]);
				r[j].add (r[i]);
			}
	}

	//jordan step (we do it forward because it doesn't matter on GF(2))
	for (i = 0; i < s; ++i)
		for (j = 0; j < i; ++j)
			if (m[j][i]) {
				m[j].add (m[i]);
				r[j].add (r[i]);
			}

	r.compute_transpose (res);
	return true;
}

void matrix::generate_random_invertible (uint size, prng & rng)
{
	matrix lt, ut;
	uint i, j;
	// random lower triagonal
	lt.resize (size);
	for (i = 0; i < size; ++i) {
		lt[i].resize (size);
		lt[i][i] = 1;
		for (j = i + 1; j < size; ++j) lt[i][j] = rng.random (2);
	}
	// random upper triagonal
	ut.resize (size);
	for (i = 0; i < size; ++i) {
		ut[i].resize (size);
		ut[i][i] = 1;
		for (j = 0; j < i; ++j) ut[i][j] = rng.random (2);
	}
	lt.mult (ut);
	// permute
	permutation p;
	p.generate_random (size, rng);
	p.permute (lt, *this);
}
