#include <iostream>
#include "testpic.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_NONE    "\x1b[30m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//#define FMT_STRING "%+06.2f "
#define FMT_STRING "\u2588\u2588"

void testpic::hasValue(float* value){
    
            for (int i=0;i<32;i++) {
                for (int j=0;j<24;j++) {
                    pixel[i][j] = value[32 * (23-j) + i];
                    float val = pixel[i][j];
                    if(val > 99.99) val = 99.99;
                if(val > 32.0){
                    //printf(ANSI_COLOR_MAGENTA FMT_STRING ANSI_COLOR_RESET, val);
                    std::cout << ANSI_COLOR_MAGENTA << FMT_STRING ANSI_COLOR_RESET ;
                }
                else if(val > 29.0){
                    //printf(ANSI_COLOR_RED FMT_STRING ANSI_COLOR_RESET, val);
                    std::cout << ANSI_COLOR_RED << FMT_STRING ANSI_COLOR_RESET ;
                }
                else if (val > 26.0){
                    //printf(ANSI_COLOR_YELLOW FMT_STRING ANSI_COLOR_YELLOW, val);
                    std::cout << ANSI_COLOR_YELLOW << FMT_STRING ANSI_COLOR_RESET ;
                }
                else if ( val > 20.0 ){
                    //printf(ANSI_COLOR_NONE FMT_STRING ANSI_COLOR_RESET, val);
                    std::cout << ANSI_COLOR_NONE << FMT_STRING ANSI_COLOR_RESET ;
                }
                else if (val > 17.0) {
                    //printf(ANSI_COLOR_GREEN FMT_STRING ANSI_COLOR_RESET, val);
                    std::cout << ANSI_COLOR_GREEN << FMT_STRING ANSI_COLOR_RESET ;
                }
                
                else if (val > 10.0) {
                    //printf(ANSI_COLOR_CYAN FMT_STRING ANSI_COLOR_RESET, val);
                    std::cout << ANSI_COLOR_CYAN << FMT_STRING ANSI_COLOR_RESET ;
                }
                else {
                    //printf(ANSI_COLOR_BLUE FMT_STRING ANSI_COLOR_RESET, val);
                    std::cout << ANSI_COLOR_BLUE << FMT_STRING ANSI_COLOR_RESET ;
                }
                
            }
            std::cout << std::endl;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(20));
        std::cout << "\x1b[33A";
        
        }
     

        
    

