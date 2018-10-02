#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/justGarble.h"

#define N_allels 10000
//N_AB
#define Size_I 8
//N, it is usually equals to Size_I
#define Size_N 8
//threshold
#define Size_T 4
//length of inputs
#define n   N_allels * (4 * Size_I)
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
    int NA[N_allels][Size_I];
    int addi[Size_I*2];
    int addo[Size_I];
    int j;
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            addi[j] = inp[i*Size_I+j];
            addi[Size_I+j] = inp[Size_I*N_allels+i*Size_I+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_I, addi, addo);
        
        for (j = 0; j < Size_I; j++)
            NA[i][j] = addo[j];
    }
    
    //computing N_a = C + D
    int Na[N_allels][Size_I];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            addi[j] = inp[2*Size_I*N_allel+i*Size_I+j];
            addi[Size_I+j] = inp[3*Size_I*N_allels+i*Size_I+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_I, addi, addo);
        
        for (j = 0; j < Size_I; j++)
            Na[i][j] = addo[j];
    }
    
    //computing N_B = A + C
    int NB[N_allels][Size_I];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            addi[j] = inp[i*Size_I+j];
            addi[Size_I+j] = inp[2*Size_I*N_allels+i*Size_I+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_I, addi, addo);
        
        for (j = 0; j < Size_I; j++)
            NB[i][j] = addo[j];
    }
    
    //computing N_b = B + D
    int Nb[N_allels][Size_I];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            addi[j] = inp[Size_I*N_allels+i*Size_I+j];
            addi[Size_I+j] = inp[3*Size_I*N_allels+i*Size_I+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_I, addi, addo);
        
        for (j = 0; j < Size_I; j++)
            Nb[i][j] = addo[j];
    }
    

    //computing NA*NB
    int NANB[N_allels][2*Size_I];
    int muli[2*Size_I];
    int mulo[2*Size_I];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            muli[j] = NA[i][j];
            muli[Size_I+j] = NB[i][j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 2*Size_I, muli, mulo);
        
        for (j = 0; j < 2*Size_I; j++)
            NANB[i][j] = mulo[j];
    }
    
    //computing N*N_AB
    int NNAB[N_allels][2*Size_I];
    int N[Size_I];
    //random N
    for (i = 0; i < Size_I; i++)
        if (Size_I%3==1)
            N[i] = zero;
        else
            N[i] = one;
    
    for (j = 0; j < Size_I; j++)
    {
        muli[Size_I+j] = N[j];
    }
    
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            muli[j] = inp[i*Size_I+j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 2*Size_I, muli, mulo);
        
        for (j = 0; j < 2*Size_I; j++)
            NNAB[i][j] = mulo[j];
    }
    
    //computing subtraction
    int sub[N_allels][2*Size_I];
    int subi[4*Size_I];
    int subo[2*Size_I];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < 2*Size_I; j++)
        {
            subi[j] = NNAB[i][j];
            subi[2*Size_I+j] = NANB[i][j];
        }
        
        SUBCircuit(&garbledCircuit, &garblingContext, 4*Size_I, subi, subo);
        
        for (j = 0; j < 2*Size_I; j++)
            sub[i][j] = subo[j];
    }
    
    //computing squre
    int s[N_allels][4*Size_I];
    int muli1[4*Size_I];
    int mulo1[4*Size_I];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < 2*Size_I; j++)
        {
            muli1[j] = sub[i][j];
            muli1[2*Size_I+j] = sub[i][j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 4*Size_I, muli1, mulo1);
        
        for (j = 0; j < 4*Size_I; j++)
            s[i][j] = mulo1[j];
    }
    
    //computing the left side
    int l[N_allels][4*Size_I+size_N+1];
    int muli2[8*Size_I];
    int mulo2[8*Size_I];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < 4*Size_I; j++)
        {
            muli2[j] = s[i][j];
        }
        for (j = 0; j < 4*Size_I-Size_N-1; j++)
            muli2[4*Size_I+j] = zero;
        
        for (j = 4*Size_I-Size_N-1; j < 4*Size_I-1; j++)
            muli2[4*Size_I+j] = N[j-(4*Size_I-Size_N-1)];
        muli2[8*Size_I-1] = zero;
        
        MULCircuit(&garbledCircuit, &garblingContext, 8*Size_I, muli2, mulo2);
        
        for (j = 4*Size_I-size_N-1; j < 8*Size_I; j++)
            l[i][j-(4*Size_I+size_N+1)] = mulo2[j];
    }
    
    
    //moving to the right side
    //computing Na*Nb
    int NaNb[N_allels][2*Size_I];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            muli[j] = Na[i][j];
            muli[Size_I+j] = Nb[i][j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 2*Size_I, muli, mulo);
        
        for (j = 0; j < 2*Size_I; j++)
            NaNb[i][j] = mulo[j];
    }
    
    //computing N_A*N_a*N_B*N_b
    int NANaNBNb[N_allels][4*Size_I];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < 2*Size_I; j++)
        {
            muli1[j] = NANB[i][j];
            muli1[2*Size_I+j] = NaNb[i][j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 4*Size_I, muli1, mulo1);
        
        for (j = 0; j < 4*Size_I; j++)
            NANaNBNb[i][j] = mulo1[j];
    }
    
    //computing the right side
    int r[N_allels][4*Size_I+size_N+1];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < 4*Size_I; j++)
        {
            muli2[j] = NANaNBNb[i][j];
        }
        for (j = 0; j < 4*Size_I-Size_T; j++)
            muli2[4*Size_I+j] = zero;
        
        //random T
        for (j = 4*Size_I-Size_T; j < 4*Size_I; j++)
            if (j%3==1)
                muli2[4*Size_I+j] = zero;
            else
                muli2[4*Size_I+j] = one;
        
        MULCircuit(&garbledCircuit, &garblingContext, 8*Size_I, muli2, mulo2);
        
        for (j = 4*Size_I-size_N-1; j < 8*Size_I; j++)
            r[i][j-(4*Size_I+size_N+1)] = mulo2[j];
    }
    
    //final result
    int ld[N_allels];
    int leqi[2*(4*Size_I+size_N+1)];
    int leqo[1];
    for (i = 0; i < N_allels; i++)
    {
        for (j = 0; j < 4*Size_I+size_N+1; j++)
        {
            leqi[j] = l[i][j];
            leqi[4*Size_I+size_N+1+j] = r[i][j];
        }
        
        LEQCircuit(&garbledCircuit, &garblingContext, 2*(4*Size_I+size_N+1), leqi, leqo);
        
        ld[i] = leqo[0];
    }
    
    //report output;
    final = &ld[0];
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

