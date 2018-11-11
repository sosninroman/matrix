#include <gtest/gtest.h>
#include "matrix.h"
#include <vector>
#include <map>

TEST(MATRIX_TEST, zero_dimentional_matrix_test)
{
    matrix::Matrix<int,0, 2> zdm;
    ASSERT_EQ(zdm,2);
    zdm = 5;
    ASSERT_EQ(zdm, 5);

    const matrix::Matrix<int,0,-1> czdm;
    ASSERT_EQ(czdm, -1);
    //czdm = 1;
    zdm = -1;
    ASSERT_EQ(zdm, czdm);
    zdm = 3;
    ASSERT_EQ(zdm, 3);

    const matrix::Matrix<int, 0, -1> czdm2;
    const matrix::Matrix<int, 0, 4> czdm3;
    ASSERT_EQ(czdm, czdm2);
    ASSERT_NE(czdm, czdm3);

    matrix::Matrix<int,0> zdm2;
    ASSERT_EQ(zdm2, 0);

    matrix::Matrix<int,0> zdm3 = 9;
    ASSERT_EQ(zdm3,9);

    ASSERT_EQ(zdm3.size(), 1);

    const matrix::Matrix<int,0> czdm4 = 8;
    matrix::Matrix<int, 0, 44> zdm4 = czdm4;
    ASSERT_EQ(zdm4, 8);
    zdm4 = 9;
    ASSERT_EQ(zdm4, 9);
    ASSERT_EQ(czdm4, 8);

    const matrix::Matrix<int,0> zdm5 = czdm4;
    ASSERT_EQ(zdm5, static_cast<int>(czdm4) );

    matrix::Matrix<int,0, 800> m1;
    matrix::Matrix<int,0> m2 = m1;
    ASSERT_EQ(m2, 800);

    matrix::Matrix<char,0> m3;
    m3 = 'a';
    ASSERT_EQ(m3,'a');
}

TEST(MATRIX_TEST, one_dimensional_matrix_test)
{
    matrix::Matrix<int,1> odm;
    odm[1] = 5;
    ASSERT_EQ(odm[1],5);

    matrix::Matrix<int,0, 6> zdm = odm[1];
    ASSERT_EQ(zdm, 5);

    zdm = 1;
    ASSERT_EQ(zdm, 1);
    ASSERT_EQ(odm[1], 5);

    ASSERT_EQ(odm.size(), 1);

    odm[2] = 4;
    ASSERT_EQ(odm.size(), 2);
    ASSERT_EQ(odm[2], 4);
    ASSERT_EQ(odm[0], 0);

    for(int i = 0; i < 10000; ++i)
    {
        if(i == 1)
            ASSERT_EQ(odm[i], 5);
        else if(i == 2)
            ASSERT_EQ(odm[i], 4);
        else
            ASSERT_EQ(odm[i], 0);
    }

}

TEST(MATRIX_TEST, two_dimensional_test)
{
    matrix::Matrix<int,2,-1> matrix;
    ASSERT_EQ(matrix.size(), 0);

    auto a = matrix[0][0];
    ASSERT_EQ(a,-1);
    ASSERT_EQ(matrix.size(), 0);

    matrix[100][100] = 314;
    ASSERT_EQ(matrix[100][100], 314);
    ASSERT_EQ(matrix.size(), 1);

    ((matrix[100][100] = 2)=4)=8;
    ASSERT_EQ(matrix[100][100], 8);
    ASSERT_EQ(matrix.size(), 1);
}

TEST(MATRIX_TEST, iterator_test)
{
    matrix::Matrix<int,1> odmatrix;
    odmatrix[1] = 5;
    odmatrix[8] = 4;
    odmatrix[2] = 9;
    odmatrix[3] = 0;

    auto itr = odmatrix.begin();
    auto end = odmatrix.end();
    ASSERT_NE(itr, end);
    std::set<int> vals;
//    while(itr != end)
//    {
        vals.insert(*itr);
//        ++itr;
//    }
    ASSERT_TRUE(vals.find(5) != vals.end() );
//    ASSERT_TRUE(vals.find(4) != vals.end() );
//    ASSERT_TRUE(vals.find(9) != vals.end() );
}
