#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/justGarble.h"

#define N_allels 10000
//N_AB
#define Size_A 8
//N_Ab
#define Size_B 8
//N_aB
#define Size_C 8
//N_ab
#define Size_D 8
//N
#define Size_N 8
//length of inputs
#define n   N_allels * (Size_A + Size_B + Size_C + Size_D)
//length of output
#define m    1


int *final;

int main(int argc, char **argv) {
    srand(time(NULL));
    GarbledCircuit garbledCircuit;
    GarblingContext garblingContext;
    
    //Set up circuit parameters
    int q, r;
    q = 10000;
    r = 10000;
    
    //Set up input and output tokens/labels.
    block *labels = (block*) malloc(sizeof(block) * 2 * n);
    block *outputbs = (block*) malloc(sizeof(block) * m);
    
    OutputMap outputMap = outputbs;
    InputLabels inputLabels = labels;
    
    //Actually build a circuit. Alternatively, this circuit could be read
    //from a file.
    createInputLabels(labels, n);
    createEmptyGarbledCircuit(&garbledCircuit, n, m, q, r, inputLabels);
    startBuilding(&garbledCircuit, &garblingContext);
    
    //all inputs in order of A, B, C, D
    int zero = fixedZeroWire(&garbledCircuit, &garblingContext);
    int one = fixedOneWire(&garbledCircuit, &garblingContext);
    int inp[n];
    int i;
    
    //random inputs
    for (i = 0; i < n; i++)
        if (n%3==1)
            inp[i] = zero;
        else
            inp[i] = one;
    
    //computing N_A = A + B
    int NA[N_allels][Size_A];
    int addi[Size_A*2];
    int addo[Size_A];
    int j;
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_A; j++)
        {
            addi[j] = inp[i*Size_A+j];
            addi[Size_A+j] = inp[Size_A*N_allels+i*Size_A+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_A, addi, addo);
        
        for (j = 0; j < Size_A; j++)
            NA[i][j] = addo[j];
    }
    
    //computing N_a = C + D
    int Na[N_allels][Size_A];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_A; j++)
        {
            addi[j] = inp[2*Size_A*N_allel+i*Size_A+j];
            addi[Size_A+j] = inp[3*Size_A*N_allels+i*Size_A+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_A, addi, addo);
        
        for (j = 0; j < Size_A; j++)
            Na[i][j] = addo[j];
    }
    
    //computing N_B = A + C
    int NB[N_allels][Size_A];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_A; j++)
        {
            addi[j] = inp[i*Size_A+j];
            addi[Size_A+j] = inp[2*Size_A*N_allels+i*Size_A+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_A, addi, addo);
        
        for (j = 0; j < Size_A; j++)
            NB[i][j] = addo[j];
    }
    
    //computing N_b = B + D
    int Nb[N_allels][Size_A];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_A; j++)
        {
            addi[j] = inp[Size_A*N_allels+i*Size_A+j];
            addi[Size_A+j] = inp[3*Size_A*N_allels+i*Size_A+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_A, addi, addo);
        
        for (j = 0; j < Size_A; j++)
            Nb[i][j] = addo[j];
    }
    

    //computing NA*NB
    int NANB[N_allels][Size_A*2];
    int muli[2*Size_A];
    int mulo[2*Size_A];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_A; j++)
        {
            muli[j] = NA[i][j];
            muli[Size_A+j] = NB[i][j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 2*Size_A, muli, mulo);
        
        for (j = 0; j < Size_A*2; j++)
            NANB[i][j] = mulo[j];
    }
    
    //computing N*N_AB
    int NNAB[N_allels][Size_A*2];
    int N[Size_A];
    //random N
    for (i = 0; i < Size_A; i++)
        if (Size_A%3==1)
            N[i] = zero;
        else
            N[i] = one;
    
    for (j = 0; j < Size_A; j++)
    {
        muli[Size_A+j] = N[j];
    }
    
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_A; j++)
        {
            muli[j] = inp[i*Size_A+j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 2*Size_A, muli, mulo);
        
        for (j = 0; j < Size_A*2; j++)
            NNAB[i][j] = mulo[j];
    }
    
    //computing subtraction
    int sub[N_allels][Size_A*2];
    int subi[4*Size_A];
    int subo[2*Size_A];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < 2*Size_A; j++)
        {
            subi[j] = NNAB[i][j];
            subi[2*Size_A+j] = NANB[i][j];
        }
        
        SUBCircuit(&garbledCircuit, &garblingContext, 4*Size_A, subi, subo);
        
        for (j = 0; j < Size_A*2; j++)
            sub[i][j] = subo[j];
    }
    
    //computing squre
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < 2*Size_A; j++)
        {
            muli[j] = sub[i][j];
            muli[2*Size_A+j] = sub[i][j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 4*Size_A, muli, mulo);
        
        for (j = 0; j < Size_A*2; j++)
            sub[i][j] = mulo[j];
    }
    
    
    //report output;
    final = &max[0];
    finishBuilding(&garbledCircuit, &garblingContext, outputMap, final);
    int TIMES1 = 2;
    long int timeGarble[TIMES];
    long int timeEval[TIMES];
    double timeGarbleMedians[TIMES];
    double timeEvalMedians[TIMES];
    for (j = 0; j < TIMES; j++) {
        for (i = 0; i < TIMES; i++) {
            timeGarble[i] = garbleCircuit(&garbledCircuit, inputLabels, outputMap);
            timeEval[i] = timedEval(&garbledCircuit, inputLabels);
        }
        timeGarbleMedians[j] = ((double) median(timeGarble, TIMES))/ garbledCircuit.q;
        timeEvalMedians[j] = ((double) median(timeEval, TIMES))/ garbledCircuit.q;
    }
    double garblingTime = doubleMean(timeGarbleMedians, TIMES);
    double evalTime = doubleMean(timeEvalMedians, TIMES);
    printf("%lf %lf\n", garblingTime, evalTime);
    printf("%d\n", garbledCircuit.q);
    printf("%d\n", garbledCircuit.r);
    printf("%d\n", counter);
    return 0;
    
}

