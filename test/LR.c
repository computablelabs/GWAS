#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../include/justGarble.h"

//dimension
#define D 30

//length of numbers
#define L 16

//Range of the numbers for precomputed values
#define R 12

//length of inputs w, x, and b
#define n 2*L*(D+1)

//length of output e^{wx+b}
#define m 1


int *final;

int main(int argc, char **argv) {
    srand(time(NULL));
    GarbledCircuit garbledCircuit;
    GarblingContext garblingContext;
    
    //Set up circuit parameters
    int q, r;
    q = 165800000;
    r = 300000000;
   
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
    
    //define 0 and 1 wires
    int zero = fixedZeroWire(&garbledCircuit, &garblingContext);
    int one = fixedOneWire(&garbledCircuit, &garblingContext);

    //generate random inputs
    int inp[n];
    int i, j;

    for (i = 0; i < n; i++)
	if (i%2 == 0)
		inp[i] = zero;
	else 
		inp[i] = one;

    //inner product multiplication
    int inpMUL[L*2];
    int outMUL[L*2];
    int inpADD[L*4];
    int outADD[L*2];
    for (j = 0; j < L*2; j++)
	outADD[j] = zero;
   
    for (i = 0; i < D; i++)
    {
	for (j = 0; j < L; j++)
		inpMUL[j] = inp[i*L+j];
	for (j = 0; j < L; j++)
		inpMUL[j+L] = inp[L*D+i*L+j];
	MULCircuit(&garbledCircuit, &garblingContext, L*2, inpMUL, outMUL);

        for (j = 0; j < L*2; j++)
		inpADD[j] = outADD[j];
	for (j = 0; j < L*2; j++)
		inpADD[L*2+j] = outMUL[j];
        ADDCircuit(&garbledCircuit, &garblingContext, L*4, inpADD, outADD);
    }

    //addition with b
    for (j = 0; j < L*2; j++)
	inpADD[j] = outADD[j];
    for (j = 0; j < L*2; j++)
	inpADD[L*2+j] = inp[2*L*D+j];
    ADDCircuit(&garbledCircuit, &garblingContext, L*4, inpADD, outADD);

    //a pre-defined value for the output, in this implementation I do not consider latency of retreving the precomputed value from database
    int out[L*2];
    for (i = 0; i < L*2; i++)
	if (i%2 == 0)
		out[i] = zero;
	else 
		out[i] = one;

   //multiplexer implementation 
    int outnot[L*2];
    NOTCircuit(&garbledCircuit, &garblingContext, L*2, outADD, outnot);

    //this part really depends on the rang of the numbers you want to check
    int bin[R];
    int revbin[R];
    int temp, k;
    int inpAND[2];
    int outAND[1];
    int inpAND1[L*4];
    int outAND1[L*2];
    for (i = 0; i < L*2; i++)
      	inpAND1[i] = out[i];

    int inpXOR[L*4];
    int outXOR[L*2];

    for (i = 0; i < pow(2, R); i++)
    {
	for (j = 0; j < R ; j++)
	{
		bin[j] = 0;
		revbin[j] = 0;
	}
        j = 0;
    	temp = i;
	while (temp > 0)
	{
		bin[j] = temp % 2; 
        	temp = temp / 2; 
        	j++; 
	}
	j--;
	k = 0;
	while (j > 0)
    	{
		revbin[k] = bin[j];
		j--;
		k++;
	}
	
        if (revbin[0] == 0)
		inpAND[0] = outnot[L*2-1];
	else	
		inpAND[0] = outADD[L*2-1];

        if (revbin[1] == 0)
		inpAND[1] = outnot[L*2-2];
	else	
		inpAND[1] = outADD[L*2-2];

        ANDNewCircuit(&garbledCircuit, &garblingContext, 2, inpAND, outAND);

        for (j = 2; j < R; j++)
	{
        	inpAND[0] = outAND[0];

                if (revbin[j] == 0)
			inpAND[1] = outnot[L*2-j-1];
		else	
			inpAND[1] = outADD[L*2-j-1];

                ANDNewCircuit(&garbledCircuit, &garblingContext, 2, inpAND, outAND);
	}

        for (j = 0; j < L*2; j++)
		inpAND1[L*2+j] = outAND[0];

        ANDNewCircuit(&garbledCircuit, &garblingContext, L*4, inpAND1, outAND1);

	if (i == 0)
	{
		for (j = 0; j < L*2; j++)
			inpXOR[j] = one;//outAND1[j];
	}
	else if (i == 1)
	{
		for (j = 0; j < L*2; j++)
			inpXOR[L*2+j] = zero;//outAND1[j];
  		XORCircuit(&garbledCircuit, &garblingContext, L*4, inpXOR, outXOR);
	}
	else
	{
		for (j = 0; j < L*2; j++)
			inpXOR[j] = one;//outAND1[j];
		for (j = 0; j < L*2; j++)
			inpXOR[L*2+j] = outXOR[j];
  		XORCircuit(&garbledCircuit, &garblingContext, L*4, inpXOR, outXOR);
        } 
    }

    //report output;
    final = &inp[0];
    finishBuilding(&garbledCircuit, &garblingContext, outputMap, final);
    int TIMES1 = 10;
    long int timeGarble[TIMES1];
    long int timeEval[TIMES1];
    double timeGarbleMedians[TIMES1];
    double timeEvalMedians[TIMES1];
    for (j = 0; j < TIMES1; j++) {
        for (i = 0; i < TIMES1; i++) {
            timeGarble[i] = garbleCircuit(&garbledCircuit, inputLabels, outputMap);
            timeEval[i] = timedEval(&garbledCircuit, inputLabels);
        }
        timeGarbleMedians[j] = ((double) median(timeGarble, TIMES1))/ garbledCircuit.q;
        timeEvalMedians[j] = ((double) median(timeEval, TIMES1))/ garbledCircuit.q;
    }
    double garblingTime = doubleMean(timeGarbleMedians, TIMES1);
    double evalTime = doubleMean(timeEvalMedians, TIMES1);
    printf("Garbling time:  %lf  Evaluation time:  %lf\n", garblingTime, evalTime);
    printf("Total number of gates:  %d\n", garbledCircuit.q);
    printf("Total number of wires:  %d\n", garbledCircuit.r);
    printf("Number of XOR gates:  %d\n", counter);
    return 0;
}

