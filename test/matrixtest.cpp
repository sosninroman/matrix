#include <gtest/gtest.h>
#include "matrix.h"

TEST(MATRIX_TEST, zero_dimentional_matrix_test)
{
    matrix::Matrix<int,0, 2> zdm;
    ASSERT_EQ(zdm,2);
    zdm = 5;
    ASSERT_EQ(zdm, 5);

    const matrix::Matrix<int,0,3> czdm;
    ASSERT_EQ(czdm, 3);
    //czdm = 1;
    zdm = 3;
    ASSERT_EQ(zdm, czdm);

    const matrix::Matrix<int, 0, 3> czdm2;
    const matrix::Matrix<int, 0, 4> czdm3;
    ASSERT_EQ(czdm, czdm2);
    ASSERT_NE(czdm, czdm3);

    matrix::Matrix<int,0> zdm2;
    ASSERT_EQ(zdm2, 0);

    matrix::Matrix<int,0> zdm3 = 9;
    ASSERT_EQ(zdm3,9);
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
}
