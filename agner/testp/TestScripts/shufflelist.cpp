#include <stdio.h>
#include <stdlib.h>


/***********************************************************************
Define random number generator class
***********************************************************************/

class CRandomMersenne {                // Encapsulate random number generator
    // Define constants for type MT11213A:
#define MERS_N   351
#define MERS_M   175
#define MERS_R   19
#define MERS_U   11
#define MERS_S   7
#define MERS_T   15
#define MERS_L   17
#define MERS_A   0xE4BD75F5
#define MERS_B   0x655E5280
#define MERS_C   0xFFD58000

public:
    CRandomMersenne(int seed) {         // Constructor
        RandomInit(seed);}
    void RandomInit(int seed);          // Re-seed
    int IRandom (int min, int max);     // Output random integer
    double Random();                    // Output random float
    unsigned int BRandom();             // Output random bits
private:
    void Init0(int seed);               // Basic initialization procedure
    unsigned int mt[MERS_N];            // State vector
    int mti;                            // Index into mt
};    


class StochasticLib1 : public CRandomMersenne {
    // This class encapsulates the random variate generating functions.
public:
    StochasticLib1 (int seed) : CRandomMersenne(seed) {}; // Constructor
    void Shuffle(int * list, int min, int n); // Shuffle integers
};


/***********************************************************************
  Allocate memory buffers. Not thread safe
***********************************************************************/
const int align_by = 64;
char * buffer = 0;
char * buffer_aligned = 0;
int  * ShuffledList = 0;

extern "C" char * AllocateBuffers(size_t bufferlen, int listlen) {
    if (listlen) {    
        ShuffledList = new int[listlen];
        if (!ShuffledList) {
            printf("\nAllocation failed for size %i\n", (int)listlen);
            exit(1);
        }
    }
    buffer = new char[bufferlen + align_by];
    if (!buffer) {
        printf("\nAllocation failed for size %i\n", (int)bufferlen);
        exit(1);
    }
    buffer_aligned = (char*)(((size_t)buffer+align_by-1) & -(size_t)align_by);
    return buffer;
}

extern "C" void DeAllocateBuffers() {
    if (buffer) delete[] buffer;
    if (ShuffledList) delete[] ShuffledList;
}


/***********************************************************************
  Allocate memory buffers. Thread safe
***********************************************************************/
extern "C" char * AllocateBufferT(size_t bufferlen) {
    char * buff;
    buff = new char[bufferlen];
    if (!buff) {
        fprintf(stderr, "\nAllocation failed for size %i\n", (int)bufferlen);
        printf("\nAllocation failed for size %i\n", (int)bufferlen);
        exit(1);
    }
    // fprintf(stderr, "\n>> allocate buff = 0x%lX, size 0x%lX\n", (long int)buff, (long int)bufferlen);
    // memset(buff, bufferlen, 1);
    return buff;
}

extern "C" void DeAllocateBufferT(char * buff) {    
    // fprintf(stderr, "\n>> free buff = 0x%lX\n", (long int)buff);
    if (buff) delete[] buff;
}


/***********************************************************************
  Create linked list of pointers in shuffled order
***********************************************************************/

extern "C" char * shuffle(int listlen, int stride, int seed) {
    //fprintf(stderr, "\nshuffle listlen %i, stride %i, seed %i", listlen, stride, seed);
    int i;
    StochasticLib1 ran(seed);
    size_t bufferlen = (size_t)listlen * stride;
    // allocate buffers
    AllocateBuffers(bufferlen, listlen);
    // make sequential list
    for (i=0; i<listlen; i++) ShuffledList[i] = i;
    // shuffle list unless seed = 0
    if (seed) {    
        ran.Shuffle(ShuffledList, 0, listlen);
    }
    // make circular chain of pointers for random walk
    int last = ShuffledList[listlen-1];
    int next;
    char * p0, * p1;
    p1 = buffer_aligned + last * stride;
    for (i=0; i<listlen; i++) {
        p0 = p1;
        next = ShuffledList[i];
        p1 = buffer_aligned + next * stride;
        *(char**)p0 = p1;
    }
    return buffer_aligned;
}


/***********************************************************************
 Random number generator class member functions
***********************************************************************/

void CRandomMersenne::Init0(int seed) {
    // Seed generator
    const unsigned int factor = 1812433253UL;
    mt[0]= seed;
    for (mti=1; mti < MERS_N; mti++) {
        mt[mti] = (factor * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
    }
}

void CRandomMersenne::RandomInit(int seed) {
    // Initialize and seed
    Init0(seed);

    // Randomize some more
    for (int i = 0; i < 37; i++) BRandom();
}

unsigned int CRandomMersenne::BRandom() {
    // Generate 32 random bits
    unsigned int y;

    if (mti >= MERS_N) {
        // Generate MERS_N words at one time
        const unsigned int LOWER_MASK = (1LU << MERS_R) - 1;       // Lower MERS_R bits
        const unsigned int UPPER_MASK = 0xFFFFFFFF << MERS_R;      // Upper (32 - MERS_R) bits
        static const unsigned int mag01[2] = {0, MERS_A};

        int kk;
        for (kk=0; kk < MERS_N-MERS_M; kk++) {    
            y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
            mt[kk] = mt[kk+MERS_M] ^ (y >> 1) ^ mag01[y & 1];}

        for (; kk < MERS_N-1; kk++) {    
            y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
            mt[kk] = mt[kk+(MERS_M-MERS_N)] ^ (y >> 1) ^ mag01[y & 1];}      

        y = (mt[MERS_N-1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[MERS_N-1] = mt[MERS_M-1] ^ (y >> 1) ^ mag01[y & 1];
        mti = 0;
    }
    y = mt[mti++];

    // Tempering (May be omitted):
    y ^=  y >> MERS_U;
    y ^= (y << MERS_S) & MERS_B;
    y ^= (y << MERS_T) & MERS_C;
    y ^=  y >> MERS_L;

    return y;
}

double CRandomMersenne::Random() {
    // Output random float number in the interval 0 <= x < 1
    // Multiply by 2^(-32)
    return (double)BRandom() * (1./(65536.*65536.));
}

int CRandomMersenne::IRandom(int min, int max) {
    // Output random integer in the interval min <= x <= max
    // Relative error on frequencies < 2^-32
    if (max <= min) {
        if (max == min) return min; else return 0x80000000;
    }
    // Multiply interval with random and truncate
    int r = int((double)(unsigned int)(max - min + 1) * Random() + min); 
    if (r > max) r = max;
    return r;
}


/***********************************************************************
Shuffle function
***********************************************************************/
void StochasticLib1::Shuffle(int * list, int min, int n) {
    /*
    This function makes a list of the n numbers from min to min+n-1
    in random order.

    The parameter 'list' must be an array with at least n elements.
    The array index goes from 0 to n-1.
    */
    int i, j, swap;
    // put numbers from min to min+n-1 into list
    for (i=0, j=min; i<n; i++, j++) list[i] = j;
    // shuffle list
    for (i=0; i<n-1; i++) {
        // item number i has n-i numbers to choose between
        j = IRandom(i,n-1);
        // swap items i and j
        swap = list[j];  list[j] = list[i];  list[i] = swap;
    }
}
