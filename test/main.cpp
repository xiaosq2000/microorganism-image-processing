
/**
 * @file main.cpp
 * @author 肖书奇 (xiaosq2000@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-13
 * 
 */

#include "segmentation.hpp"

int main()
{
    Segmentation test1("../share/src/6000", "../share/dst/6000");
    test1.~Segmentation();
    Segmentation test2("../share/src/10000 up", "../share/dst/10000 up");
    test2.~Segmentation();
    Segmentation test3("../share/src/10000 down", "../share/dst/10000 down");
    test3.~Segmentation();
    return 0;
}
