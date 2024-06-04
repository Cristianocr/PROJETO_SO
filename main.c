#include <stdio.h>
#include "monteCarloAB.h"
#include "monteCarloCD_a.h"
#include "monteCarloCD_b.h"
#include "monteCarloE_a.h"
#include "monteCarloE_b.h"
#include "mcThreadA.h"
#include "mcThreadB.h"
#include "mcThreadC.h"

#include <time.h>


int main(int argc, char *argv[]) {
    /*argv[0] = "./monteCarlo1";
    argv[1] = "poligon.txt";
    argv[2] = "3";
    argv[3] = "900";
    argc = 4;*/
    //monteCarloAB(argc, argv);
    //monteCarloCD_a(argc, argv);
    //monteCarloCD_b(argc, argv);
    //monteCarloE_a(argc, argv);
    //monteCarloE_b(argc, argv);

    //mcThreadA(argc, argv);
    //mcThreadB(argc, argv);
    mcThreadC(argc, argv);


    return 0;
}
