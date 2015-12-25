#pragma once
#ifndef _MATLABFUNCTION_H_
#define _MATLABFUNCTION_H_

#include<cmath>  //exp, floor
#include<cfloat> //FLT_MIN
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
    void disp(vectorF2D &vec);

    //�Զ�ά����vec�İ�dimά����ԭ�ط�ת
    void flip(vectorF2D &vec, unsigned dim);
    //������ά����vec����dimά��ת�������
    vectorF4D flip(const vectorF4D  &vec, unsigned dim);

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

    std::vector<unsigned> size(const vectorF3D &vec);

    std::vector<unsigned> size(const vectorF4D &vec);

    void error(const char* str);

    std::vector<int> randperm(unsigned n, unsigned k=0);

    std::vector<int> linspace(int a, int b, unsigned n);

    vectorF zeros(uint a);

    vectorF2D zeros(uint a, uint b);

    vectorF3D zeros(uint a, uint b, uint c);

    vectorF4D zeros(uint a, uint b, uint c, uint d, float first = 0);

    vectorF4D zerosLike(const vectorF4D &vec);

    inline double sigm(double x);
    //��һ��Mapͼÿ�����ƫ�ú���sigmoid
    void sigm(vectorF2D &vec, float bias);

    //���ģʽ���ο�matlab
    enum Shape {
        FULL, SAME, VALID
    };
    vectorF2D conv2(const vectorF2D &A, const vectorF2D &B, Shape shape = FULL);

    //��һ�������ƽ��ֵ
    float mean(const vectorF &vec, unsigned from, unsigned len);

    //��ȡһ��һά�����е����ֵ������
    inline float max(const vectorF &vec);

    //��ȡһ��2ά�����е����ֵ�����浽1ά�����з���
    inline vectorF max(const vectorF2D &vec);

    //��ȡһ��4ά�����е����ֵ�����浽[1][1][*][*]�����з���
    vectorF4D max4D(const vectorF4D &vec);

    //��һ��4ά�����ص����ƣ�[1 1 j k] => [m n j k]
    inline vectorF4D repmat4D(const vectorF4D &vec, int m, int n);

    template<typename T>
    inline bool equal(T a, T b)
    {
        return a - b<1e-5 && b - a<1e-5;
    }

    //��haveMax�����в���maxMax(���ֵ)��Ԫ����0,��ֻ�������е����ֵ
    vectorF4D reserveMax(vectorF4D &haveMax, const vectorF4D &maxMat);

}

#ifndef _MSC_VER //���ݷ���Ŀ�������루����CB�����У�
#include"matlabFunc.cpp"
#endif

#endif //_MATLABFUNCTION_H_
