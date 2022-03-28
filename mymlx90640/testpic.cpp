#include <iostream>
#include "testpic.h"
extern bool readd;

void testpic::hasValue(float* value){
        if (readd) {
            for (int i=0;i<32;i++) {
                for (int j=0;j<32;j++) {
                    pixel[i][j] = value[32 * (23-j) + i];
                    std::cout << pixel[i][j] << std::endl;
                }
            }
            readd = !readd;
        }
        
    
}
