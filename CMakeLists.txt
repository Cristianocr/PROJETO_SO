cmake_minimum_required(VERSION 3.27)
project(SO_PROJECT C)

set(CMAKE_C_STANDARD 99)

add_executable(SO_PROJECT main.c
        monteCarloAB.c
        monteCarloAB.h
        monteCarloAB.c
        monteCarloAB.h
        monteCarloAB.c
        monteCarloAB.h
        monteCarloCD_a.c
        monteCarloCD_a.h
        apue.h
        monteCarloCD_b.h
        monteCarloCD_b.c
        monteCarloE_a.c
        monteCarloE_A.h
        monteCarloE_b.c
        monteCarloE_b.h
        monteCarloE_a.c
        monteCarloE_a.h
        mcThreadA.c
        mcThreadA.h
        mcThreadB.c
        mcThreadB.h
        mcThreadC.c
        mcThreadC.h)
