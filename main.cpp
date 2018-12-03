#include <iostream>
#include "matrix.h"

int main()
{
    matrix::Matrix<int, 2> matr;
    const int sz = 10;
    for(int i = 0; i < sz; ++i)
    {
        matr[i][i] = i;
        matr[i][sz-i-1] = sz-i-1;
    }

    for(int i = 1; i < 9; ++i)
    {
        for(int j = 1; j < 9; ++j)
            std::cout << matr[i][j] << " ";
        std::cout << std::endl;
    }

    std::cout << "size: " << matr.size() << std::endl;

    int x,y,v;
    for(auto val : matr)
    {
        std::tie(x,y,v) = val;
        std::cout << "matrix[" << x <<"][" << y << "] = " << v << std::endl;
    }
    return 0;
}
