#include "cae.h"
using namespace std;
const bool TestOut = 0;

CAE::CAE(int _ic, int _oc, int _ks, int _ps, double _noise)
	: b(mat::zeros(_oc)), c(mat::zeros(_ic)),
	w(_oc, vectorF3D(_ic, vectorF2D(_ks, vectorF(_ks)))) {
	setSrand();
	ic = _ic;
	oc = _oc;
	ks = _ks;
	ps = _ps;
	noise = (float)_noise;

	int i, j, m, n;
	for (i = 0; i < oc; ++i)
		for (j = 0; j < ic; ++j)
			for (m = 0; m < ks; ++m)
				for (n = 0; n < ks; ++n)
					w[i][j][m][n] = (randFloat() - .5f) * 60 / (oc*ks*ks);
	w_tilde = mat::flip(mat::flip(w, 1), 2);
}

void CAE::setup(int _ic, int _oc, int _ks, int _ps, double _noise) {
	*this = CAE(_ic, _oc, _ks, _ps, _noise);
}

void CAE::train(const vectorF4D &x, const OPTS &opts) {
	PARA para = check(x, opts);
	L = mat::zeros(opts.numepochs * (int)para.bnum);
	int t_start;
	vector<int> idx;
	vectorF4D batch_x = mat::zeros(para.bsze, mat::size(x, 2)
		, mat::size(x, 3), mat::size(x, 4));
	for (int i = 0; i < opts.numepochs; ++i) {
		cout << ">>> epoch " << i << "/" << opts.numepochs << " \t";
		t_start = clock(); //��ʼ��ʱ
		if (opts.shuffle==1)
			idx = mat::randperm(para.pnum);
		else
			idx = mat::linspace(1, para.pnum, para.pnum);
		for (int j = 0; j < para.bnum; ++j) {
			if (TestOut)cout << "j = " << j << "/" << para.bnum << " \t";
			for (int k = 0; k < para.bsze; ++k)
				batch_x[k] = x[idx[j * para.bsze + k]];
			ffbp(batch_x, para);
			update(opts);
			L[i*(int)para.bnum + j] = loss;
		}
		//��ʾƽ��ֵ
		cout << mat::mean(L,i*(int)para.bnum,(int)para.bnum) << "\t" << clock() - t_start << " ms"<<endl;
		t_start = clock();
	}
}

void CAE::ffbp(const vectorF4D &x, PARA &para) {
	if(TestOut)cout << "in ffbp\t" << clock() << endl;
	vectorF4D x_noise = x;
	uint i, j, m, n;
	vector<uint> x_size = mat::size(x_noise);
	for (i = 0; i < x_size[0]; ++i)
		for (j = 0; j < x_size[1]; ++j)
			for (m = 0; m < x_size[2]; ++m)
				for (n = 0; n < x_size[3]; ++n)
					if (randFloat() < noise)
						x_noise[i][j][m][n] = 0;
	if(TestOut)cout << "in up\t" << clock() << endl;
	up(x_noise, para);
	if(TestOut)cout << "in pool\t" << clock() << endl;
	pool(para);
	if(TestOut)cout << "in down\t" << clock() << endl;
	down(para);
	if(TestOut)cout << "in grad\t" << clock() << endl;
	grad(x, para);
}

void CAE::up(const vectorF4D &x, PARA &para) {
	h = mat::zeros(para.bsze, oc, para.m - ks + 1, para.m - ks + 1);
	for (int pt = 0; pt < para.bsze; ++pt)
		for (int _oc = 0; _oc < oc; ++_oc) {
			for (int _ic = 0; _ic < ic; ++_ic)
				addVector(h[pt][_oc], h[pt][_oc], mat::conv2(x[pt][_ic], w[_oc][_ic], mat::VALID));
			mat::sigm(h[pt][_oc], b[_oc]);
		}
}

void CAE::pool(const PARA &para) {
	if (ps >= 2) {
		h_pool = h_mask = mat::zeros(h); //[2 6 24 24]
		ph = mat::zeros(para.bsze, oc, (uint)para.pgrds, (uint)para.pgrds); //[2 6 2 2]
		vectorF4D grid = mat::zeros(para.bsze, oc, ps, ps); //[2 6 2 2]
		vectorF4D sparse_grid, mask, tmpMax;
		for (int i = 0; i < para.pgrds; ++i) {
			for (int j = 0; j < para.pgrds; ++j) {
				for (int pt = 0; pt < para.bsze; ++pt)
					for (int _oc = 0; _oc < oc; ++_oc)
						for (int jj = 0; jj < ps; ++jj)
							for (int ii = 0; ii < ps; ++ii)
								grid[pt][_oc][jj][ii] = h[pt][_oc][j*ps + jj][i*ps + ii];
				tmpMax = mat::max4D(grid);
				//ph[i][j] = tmpMax[0][0];
				for (int ii = 0; ii < para.bsze; ++ii)
					for (int jj = 0; jj < oc; ++jj)
						ph[ii][jj][i][j] = tmpMax[ii][jj][0][0];
				sparse_grid = grid;
				mask = mat::reserveMax(sparse_grid, tmpMax);
				for (int pt = 0; pt < para.bsze; ++pt)
					for (int _oc = 0; _oc < oc; ++_oc)
						for (int jj = 0; jj < ps; ++jj)
							for (int ii = 0; ii < ps; ++ii) {
								h_pool[pt][_oc][j*ps + jj][i*ps + ii] = sparse_grid[pt][_oc][jj][ii];
								h_mask[pt][_oc][j*ps + jj][i*ps + ii] = mask[pt][_oc][jj][ii];
							}
			}
		}
	}
	else {
		ph = h;
		h_pool = h;
		h_mask = h;
	}

}

void CAE::down(PARA &para) {
	out = mat::zeros(para.bsze, ic, para.m, para.m);
	for (int pt = 0; pt < para.bsze; ++pt)
		for (int _ic = 0; _ic < ic; ++_ic) {
			for (int _oc = 0; _oc < oc; ++_oc)
				addVector(h[pt][_ic], h[pt][_ic], mat::conv2(h_pool[pt][_oc], w_tilde[_oc][_ic], mat::FULL));
			mat::sigm(out[pt][_ic], b[_ic]);
		}
}

void CAE::grad(const vectorF4D &x, PARA &para) {
	unsigned i, j, m, n;
	vector<unsigned> sizeOut = mat::size(out);
	if (err.size() == 0 || dy.size() == 0)  //�����û��ʼ��err��dy���������С����out
		dy = err = mat::zeros(out);
	loss = 0;
	vectorF tempDc(sizeOut[0] * sizeOut[1], 0);
	for (i = 0; i < sizeOut[0]; ++i)
		for (j = 0; j < sizeOut[1]; ++j)
			for (m = 0; m < sizeOut[2]; ++m)
				for (n = 0; n < sizeOut[3]; ++n) {
					err[i][j][m][n] = out[i][j][m][n] - x[i][j][m][n];
					dy[i][j][m][n] = err[i][j][m][n] * (out[i][j][m][n] * (1 - out[i][j][m][n])) / para.bsze;
					loss += err[i][j][m][n] * err[i][j][m][n];
					tempDc[i*sizeOut[1] + j] += dy[i][j][m][n];
				}
	loss /= (2 * para.bsze); // 0.5 * sum(err[:] ^2) / bsze
	if (dc.size() == 0)
		dc = mat::zeros(c.size());
	for (i = m = 0; i < c.size(); ++i) {
		dc[i] = 0.0f;
		for (j = 0; j < (uint)para.bsze; ++j)
			dc[i] += tempDc[m++];
	}
	if (TestOut)cout << "update [dh]\t" << clock() << endl;
	//����dh
	dh = mat::zeros(h); //[2 6 24 24]
	int _pt, _oc, _ic;
	for (_pt = 0; _pt < para.bsze; ++_pt)
		for (_oc = 0; _oc < oc; ++_oc)
			for (_ic = 0; _ic < ic; ++_ic)
				addVector(dh[_pt][_oc], conv2(dy[_pt][_ic], w[_oc][_ic], mat::VALID));
	vector<uint> sizeDh = mat::size(dh);
	if (ps >= 2) {
		for (i = 0; i < sizeDh[0]; ++i)
			for (j = 0; j < sizeDh[1]; ++j)
				for (m = 0; m < sizeDh[2]; ++m)
					for (n = 0; n < sizeDh[3]; ++n)
						dh[i][j][m][n] *= h_mask[i][j][m][n];
	}
	for (i = 0; i < sizeDh[0]; ++i)
		for (j = 0; j < sizeDh[1]; ++j)
			for (m = 0; m < sizeDh[2]; ++m)
				for (n = 0; n < sizeDh[3]; ++n)
					dh[i][j][m][n] *= h[i][j][m][n] * (1 - h[i][j][m][n]);
	if (TestOut)cout << "update [db]\t" << clock() << endl;
	//����db
	if (db.size() == 0)
		db = mat::zeros(b.size());
	if (para.pgrds >= 2) {
		vectorF tempDb(sizeDh[0] * sizeDh[1], 0);
		uint c = 0;
		for (i = 0; i < sizeDh[0]; ++i)
			for (j = 0; j < sizeDh[1]; ++j, ++c)
				for (m = 0; m < sizeDh[2]; ++m)
					for (n = 0; n < sizeDh[3]; ++n)
						tempDb[c] += dh[i][j][m][n];
		for (i = c = 0; i < b.size(); ++i) {
			db[i] = 0.0f;
			for (j = 0; j < (uint)para.bsze; ++j)
				db[i] += tempDb[c++];
		}
	}
	else {
		vectorF tempDb(sizeDh[0] * sizeDh[1] * sizeDh[2] * sizeDh[3], 0);
		uint c = 0;
		for (i = 0; i < sizeDh[0]; ++i)
			for (j = 0; j < sizeDh[1]; ++j)
				for (m = 0; m < sizeDh[2]; ++m)
					for (n = 0; n < sizeDh[3]; ++n)
						tempDb[c++] += dh[i][j][m][n];
		for (i = c = 0; i < b.size(); ++i) {
			db[i] = 0.0f;
			for (j = 0; j < (uint)para.bsze; ++j)
				db[i] += tempDb[c++];
		}
	}
	if (TestOut)cout << "update [dw]\t" << clock() << endl;
	//����dw
	dw = mat::zeros(w); //[6 1 5 5]
	dy_tilde = mat::flip(mat::flip(dy, 1), 2); //[2 1 28 28]
	vectorF4D x_tilde = mat::flip(mat::flip(x, 1), 2);//[2 1 28 28]
	for (_pt = 0; _pt < para.bsze; ++_pt) //2
		for (_oc = 0; _oc < oc; ++_oc) //6
			for (_ic = 0; _ic < ic; ++_ic) { //1
				addVector(dw[_oc][_ic]
					, mat::conv2(x_tilde[_pt][_ic], dh[_pt][_oc], mat::VALID)
					, mat::conv2(dy_tilde[_pt][_ic], h_pool[_pt][_oc], mat::VALID));
			}
}

void CAE::update(const OPTS &opts) {
	unsigned sizeB = b.size(), sizeC = c.size(), i, j, m, n;
	for (i = 0; i < sizeB; ++i)
		b[i] -= opts.alpha * db[i];
	for (i = 0; i < sizeC; ++i)
		c[i] -= opts.alpha * dc[i];
	vector<uint> sizeW = mat::size(w);
	for (i = 0; i < sizeW[0]; ++i)
		for (j = 0; j < sizeW[1]; ++j)
			for (m = 0; m < sizeW[2]; ++m)
				for (n = 0; n < sizeW[3]; ++n)
					w[i][j][m][n] -= opts.alpha * dw[i][j][m][n];
	w_tilde = mat::flip(mat::flip(w, 1), 2);
}

inline PARA CAE::check(const vectorF4D &x, const OPTS &opts) {
	if (x.size() == 0)
		mat::error("please init input data first!");
	PARA para;
	para.m = mat::size(x, 4);
	para.pnum = mat::size(x, 1);
	para.pgrds = (float)(para.m - ks + 1) / ps;
	para.bsze = opts.batchsize;
	para.bnum = (float)para.pnum / para.bsze;
	if (mat::size(x, 2) != ic)
		mat::error("number of input chanels doesn't match.");
	if (ks > para.m)
		mat::error("too large kernel.");
	if (floor(para.pgrds) < para.pgrds)
		mat::error("sides of hidden representations should be divisible by pool size.");
	if (floor(para.bnum) < para.bnum)
		mat::error("number of data points should be divisible by batch size.");
	return para;
}
