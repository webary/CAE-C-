#pragma once
#ifndef _MATLABFUNCTION_H_
#define _MATLABFUNCTION_H_

#include<ctime>
#include<vector>
#include<iostream>
#include<algorithm>

typedef unsigned uint;
typedef std::vector<float> vectorF;
typedef std::vector<vectorF> vectorF2D;
typedef std::vector<vectorF2D> vectorF3D;
typedef std::vector<vectorF3D> vectorF4D;

//ʵ��matlab�г�������
namespace mat
{
//��ʾһ����ά����
void disp(vectorF2D &vec)
{
		unsigned size[2] = { vec.size(), vec[0].size() }, i, j;
		for (i = 0; i < size[0]; ++i) {
			for (j = 0; j < size[1]; ++j)
				std::cout << vec[i][j] << " ";
			std::cout << std::endl;
		}
}
//�Զ�ά����vec�İ�dimά����ԭ�ط�ת
void flip(vectorF2D &vec, unsigned dim)
{
	unsigned size[2] = { vec.size(), vec[0].size() }, i, j, half;
	if (dim == 1) {
		half = size[0] / 2;
		for (i = 0; i < half; ++i)
			for (j = 0; j < size[1]; ++j)
				std::swap(vec[i][j], vec[size[0] - i - 1][j]);
	} else if (dim == 2) {
		half = size[1] / 2;
		for (i = 0; i < size[0]; ++i)
			for (j = 0; j < half; ++j)
				std::swap(vec[i][j], vec[i][size[1] - j - 1]);
	}
}
//������ά����vec����dimά��ת�������
vectorF4D flip(const vectorF4D  &vec, unsigned dim)
{
	if (dim != 1 && dim != 2)
		return vec;
	vectorF4D myVec = vec;
	int size[2] = { myVec.size(), myVec[0].size() }, i, j;
	for (i = 0; i < size[0]; ++i)
		for (j = 0; j < size[1]; ++j)
			flip(myVec[i][j], dim);
	return myVec;
}

//����vec�ĵ�dimά�Ĵ�С(��1��ʼ,���֧��4ά)
template<typename T>
inline unsigned size(const T &vec, int dim)
{
	if (dim == 1)
		return vec.size();
	if (dim == 2)
		return vec[0].size();
	if (dim == 3)
		return vec[0][0].size();
	if (dim == 4)
		return vec[0][0][0].size();
	return 0;
}

inline std::vector<unsigned> size(const vectorF4D &vec)
{
	std::vector<unsigned> sizeMat(4);
	sizeMat[0] = vec.size();
	sizeMat[1] = vec[0].size();
	sizeMat[2] = vec[0][0].size();
	sizeMat[3] = vec[0][0][0].size();
	return sizeMat;
}

inline void error(const char* str)
{
	std::cout << str << std::endl;
	exit(1);
}

inline std::vector<int> randperm(unsigned n)
{
	std::vector<int> randp(n);
	for (unsigned i = 0; i < n; ++i)
		randp[i] = i;
	std::random_shuffle(randp.begin(), randp.end()); //algorithm
	return randp;
}

template<typename T>
inline std::vector<T> linspace(const T &a, const T &b, unsigned n)
{
	std::vector<T> randp(n);
	T step = (b - a) / (n - 1);
	for (unsigned i = 0; i < n; ++i)
		randp[i] = a + i * step;
	random_shuffle(randp.begin(), randp.end());
	return randp;
}

inline vectorF zeros(uint a)
{
	return vectorF(a, 0);
}

inline vectorF2D zeros(uint a, uint b)
{
	return vectorF2D(a, vectorF(b, 0));
}

inline vectorF3D zeros(uint a, uint b, uint c)
{
	return vectorF3D(a, vectorF2D(b, vectorF(c, 0)));
}

inline vectorF4D zeros(uint a, uint b, uint c, uint d)
{
	return vectorF4D(a, vectorF3D(b, vectorF2D(c, vectorF(d, 0))));
}

inline vectorF4D zeros(const vectorF4D vec)
{
	return vectorF4D(vec.size(), vectorF3D(vec[0].size(), vectorF2D(vec[0][0].size(), vectorF(vec[0][0][0].size(), 0))));
}

inline double sigm(double x)
{
	return 1.0 / (1.0 + exp(-x));
}
//��һ��Mapͼÿ�����ƫ�ú���sigmoid
inline void sigm(vectorF2D &vec, float bias)
{
	for (uint i = 0; i<vec.size(); ++i)
		for (uint j = 0; j<vec[i].size(); ++j)
			vec[i][j] = (float)sigm(vec[i][j] + bias);
}
//���ģʽ���ο�matlab
enum Shape {
	FULL, SAME, VALID
};
vectorF2D conv2(const vectorF2D &A, const vectorF2D &B, Shape shape = FULL)
{
	unsigned sizeA[2] { A.size() ,A[0].size() }, sizeB[2] = { B.size() ,B[0].size() };
	vectorF2D result = zeros(sizeA[0] + sizeB[0] - 1, sizeA[1] + sizeB[1] - 1);
	unsigned i, j, m, n;
	for (i = 0; i < sizeA[0]; ++i)
		for (j = 0; j < sizeA[1]; ++j)
			for (m = 0; m < sizeB[0]; ++m)
				for (n = 0; n < sizeB[1]; ++n)
					result[i + m][j + n] += A[i][j] * B[m][n];
	if (shape == FULL)
		return result;
	unsigned delta[2] = { 0 }; //x,y�����ƫ����
	unsigned newSize[2] = { 0 }; //�´�С
	if (shape == SAME) {
		delta[0] = sizeB[0] / 2;
		delta[1] = sizeB[1] / 2;
		newSize[0] = sizeA[0];
		newSize[1] = sizeA[1];
	} else if (shape == VALID) {
		delta[0] = sizeB[0] - 1;
		delta[1] = sizeB[1] - 1;
		newSize[0] = sizeA[0] - sizeB[0] + 1;
		newSize[1] = sizeA[1] - sizeB[1] + 1;
	}
	vectorF2D res = zeros(newSize[0], newSize[1]);
	for (i = 0; i < newSize[0]; ++i)
		for (j = 0; j < newSize[1]; ++j)
			res[i][j] = result[i + delta[0]][j + delta[1]];
	return res;
}

float mean(const vectorF &vec)
{
	float meanValue = 0;
	for (unsigned i = 0; i < vec.size(); ++i)
		meanValue += vec[i];
	return meanValue / vec.size();
}

//��ȡһ��һά�����е����ֵ������
inline float max(const vectorF &vec)
{
	unsigned v_size = vec.size();
	if (v_size < 1)
		return 0;
	float maxValue = vec[0];
	for (unsigned i = 1; i < v_size; ++i)
		if (maxValue < vec[i])
			maxValue = vec[i];
	return maxValue;
}

//��ȡһ��2ά�����е����ֵ�����浽1ά�����з���
inline vectorF max(const vectorF2D &vec)
{
	vectorF maxMat = vec[0];
	unsigned v_size[2] = { vec.size(), vec[0].size() };
	for (unsigned i = 1; i < v_size[0]; ++i)
		for (unsigned j = 0; j < v_size[1]; ++j)
			if (maxMat[j] < vec[i][j])
				maxMat[j] = vec[i][j];
	return maxMat;

}

//��ȡһ��4ά�����е����ֵ�����浽[1][1][*][*]�����з���
inline vectorF4D max4D(const vectorF4D &vec)
{
	unsigned i, j, m, n;
	const std::vector<unsigned> v_size = mat::size(vec);
	vectorF4D maxMat = zeros(1,1,v_size[2],v_size[3]);
	maxMat[0][0] = vec[0][0];
	for ( i = 0; i < v_size[0]; ++i)
		for (j = 0; j < v_size[1]; ++j)
			for (m = 0; m < v_size[2]; ++m)
				for (n = 0; n < v_size[3]; ++n)
					if (maxMat[0][0][m][n] < vec[i][j][m][n])
						maxMat[0][0][m][n] = vec[i][j][m][n];
	return maxMat;

}

//��һ��4ά�����ص����ƣ�[1 1 j k] => [m n j k]
inline vectorF4D repmat4D(const vectorF4D &vec, int m, int n)
{
	unsigned i, j;
	const std::vector<unsigned> v_size = mat::size(vec);
	vectorF4D repMat = zeros(m, n, v_size[2], v_size[3]);
	for (i = 0; i < v_size[0]; ++i)
		for (j = 0; j < v_size[1]; ++j)
			repMat[i][j] = vec[0][0];
	return repMat;
}

template<typename T>
inline bool equal(T a, T b)
{
	return a-b<1e-5 && b-a<1e-5;
}

//��haveMax�����в���maxMax(���ֵ)��Ԫ����0,��ֻ�������е����ֵ
inline vectorF4D reserveMax(vectorF4D &haveMax, const vectorF4D &maxMat)
{
	unsigned i, j, m, n;
	vectorF4D mask(mat::zeros(haveMax)); //���ֵ��λ����1��ʾ
	const std::vector<unsigned> v_size = mat::size(haveMax);
	for (i = 0; i < v_size[0]; ++i)
		for (j = 0; j < v_size[1]; ++j)
			for (m = 0; m < v_size[2]; ++m)
				for (n = 0; n < v_size[3]; ++n)
					if (equal(haveMax[i][j][m][n], maxMat[0][0][m][n]))
						mask[i][j][m][n] = 1;  //������ø�λ�õ�maskΪ1
					else
						haveMax[i][j][m][n] = 0; //��������ԭʼ�����и�λ����0
	return mask;
}

}
#endif //_MATLABFUNCTION_H_