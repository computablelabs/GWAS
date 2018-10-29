#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/justGarble.h"

//two main parameters of LD test
#define M_allels 10000

//N_AB, N_Ab, N_aB, N_ab
#define Size_I 8

//N, N in our case is 200 (8), 400 (9), 800 (10), 1600 (11)
#define Size_N 8

//threshold
#define Size_T 4

//length of inputs
#define n   M_allels * (4 * Size_I)

//length of output
#define m    1


int *final;

int main(int argc, char **argv) {
    srand(time(NULL));
    GarbledCircuit garbledCircuit;
    GarblingContext garblingContext;
    
    //Set up circuit parameters
    int q, r;
    q = 90000000;
    r = 90000000;
    
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
   
    //computing N_A = N_AB + N_Ab
    int NA[M_allels][Size_I];
    int addi[Size_I*2];
    int addo[Size_I];
    int j;
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            addi[j] = inp[i*Size_I+j];
            addi[Size_I+j] = inp[Size_I*M_allels+i*Size_I+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_I, addi, addo);
        
        for (j = 0; j < Size_I; j++)
            NA[i][j] = addo[j];
    }
    
    //computing N_a = N_aB + N_ab
    int Na[M_allels][Size_I];
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            addi[j] = inp[2*Size_I*M_allels+i*Size_I+j];
            addi[Size_I+j] = inp[3*Size_I*M_allels+i*Size_I+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_I, addi, addo);
        
        for (j = 0; j < Size_I; j++)
            Na[i][j] = addo[j];
    }
    
    //computing N_B = N_AB + N_aB
    int NB[M_allels][Size_I];
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            addi[j] = inp[i*Size_I+j];
            addi[Size_I+j] = inp[2*Size_I*M_allels+i*Size_I+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_I, addi, addo);
        
        for (j = 0; j < Size_I; j++)
            NB[i][j] = addo[j];
    }
    
    //computing N_b = N_Ab + N_ab
    int Nb[M_allels][Size_I];
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            addi[j] = inp[Size_I*M_allels+i*Size_I+j];
            addi[Size_I+j] = inp[3*Size_I*M_allels+i*Size_I+j];
        }
        
        ADDCircuit(&garbledCircuit, &garblingContext, 2*Size_I, addi, addo);
        
        for (j = 0; j < Size_I; j++)
            Nb[i][j] = addo[j];
    }
    

    //computing NA*NB
    int NANB[M_allels][2*Size_I];
    int muli[2*Size_I];
    int mulo[2*Size_I];
    for (i = 0; i < M_allels; i++)
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
    int NNAB[M_allels][2*Size_N];
    int N[Size_N];
    int muli0[2*Size_N];
    int mulo0[2*Size_N];
    //random N
    for (i = 0; i < Size_N; i++)
        if (Size_N%3==1)
            N[i] = zero;
        else
            N[i] = one;
    
    for (j = 0; j < Size_N; j++)
    {
        muli0[j] = N[j];
    }
    
    for (j = Size_N; j < Size_N+(Size_N-Size_I); j++)
        muli0[j] = zero;
    
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < Size_I; j++)
        {
            muli0[Size_N+(Size_N-Size_I)+j] = inp[i*Size_I+j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 2*Size_N, muli0, mulo0);
        
        for (j = 0; j < Size_I+Size_N; j++)
            NNAB[i][j] = mulo0[j+(Size_N-Size_I)];
    }
    
    //computing subtraction
    int sub[M_allels][Size_I+Size_N];
    int subi[2*(Size_I+Size_N)];
    int subo[Size_I+Size_N];
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < Size_I+Size_N; j++)
            subi[j] = NNAB[i][j];
        
        for (j = Size_I+Size_N; j < Size_I+Size_N+(Size_I+Size_N-2*Size_I); j++)
            subi[j] = zero;
        
        for (j = 0; j < 2*Size_I; j++)
            subi[Size_I+Size_N+(Size_I+Size_N-2*Size_I)+j] = NANB[i][j];
        
        SUBCircuit(&garbledCircuit, &garblingContext, 2*(Size_I+Size_N), subi, subo);
        
        for (j = 0; j < Size_I+Size_N; j++)
            sub[i][j] = subo[j];
    }
   
    //computing squre
    int s[M_allels][2*(Size_I+Size_N)];
    int muli1[2*(Size_I+Size_N)];
    int mulo1[2*(Size_I+Size_N)];
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < Size_I+Size_N; j++)
        {
            muli1[j] = sub[i][j];
            muli1[Size_I+Size_N+j] = sub[i][j];
        }
        
        MULCircuit(&garbledCircuit, &garblingContext, 2*(Size_I+Size_N), muli1, mulo1);
        
        for (j = 0; j < 2*(Size_I+Size_N); j++)
            s[i][j] = mulo1[j];
    }
    
    //computing the left side
    int left[M_allels][2*(Size_I+Size_N)+Size_N+1];
    int muli2[4*(Size_I+Size_N)];
    int mulo2[4*(Size_I+Size_N)];
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < 2*(Size_I+Size_N); j++)
        {
            muli2[j] = s[i][j];
        }
        for (j = 0; j < 2*(Size_I+Size_N)-Size_N-1; j++)
            muli2[2*(Size_I+Size_N)+j] = zero;
        
        for (j = 2*(Size_I+Size_N)-Size_N-1; j < 4*(Size_I+Size_N)-1; j++)
            muli2[2*(Size_I+Size_N)+j] = N[j-(2*(Size_I+Size_N)-Size_N-1)];
        muli2[4*(Size_I+Size_N)-1] = zero;
        
        MULCircuit(&garbledCircuit, &garblingContext, 4*(Size_I+Size_N), muli2, mulo2);
        
        for (j = 2*(Size_I+Size_N)-Size_N-1; j < 4*(Size_I+Size_N); j++)
            left[i][j-(2*(Size_I+Size_N)-Size_N-1)] = mulo2[j];
    }
    
    
    //moving to the right side
    //computing Na*Nb
    int NaNb[M_allels][2*Size_I];
    for (i = 0; i < M_allels; i++)
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
    int NANaNBNb[M_allels][4*Size_I];
    for (i = 0; i < M_allels; i++)
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
    int right[M_allels][2*(Size_I+Size_N)+Size_N+1];
    int muli3[8*Size_I];
    int mulo3[8*Size_I];
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < 4*Size_I; j++)
        {
            muli3[j] = NANaNBNb[i][j];
        }
        for (j = 0; j < 4*Size_I-Size_T; j++)
            muli3[4*Size_I+j] = zero;
        
        //random T
        for (j = 4*Size_I-Size_T; j < 4*Size_I; j++)
            if (j%3==1)
                muli3[4*Size_I+j] = zero;
            else
                muli3[4*Size_I+j] = one;
        
        MULCircuit(&garbledCircuit, &garblingContext, 8*Size_I, muli3, mulo3);
        
        for (j = 8*Size_I-(2*(Size_I+Size_N)+Size_N+1); j < 8*Size_I; j++)
            right[i][j-(8*Size_I-(2*(Size_I+Size_N)+Size_N+1))] = mulo3[j];
    }
   
    //final result
    int ld[M_allels];
    int leqi[2*(2*(Size_I+Size_N)+Size_N+1)];
    int leqo[1];
    for (i = 0; i < M_allels; i++)
    {
        for (j = 0; j < 2*(Size_I+Size_N)+Size_N+1; j++)
        {
            leqi[j] = left[i][j];
            leqi[2*(Size_I+Size_N)+Size_N+1+j] = right[i][j];
        }
        
        LEQCircuit(&garbledCircuit, &garblingContext, 2*(2*(Size_I+Size_N)+Size_N+1), leqi, leqo);
        
        ld[i] = leqo[0];
    }

    
    //report output;
    final = &ld[0];
    finishBuilding(&garbledCircuit, &garblingContext, outputMap, final);
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

