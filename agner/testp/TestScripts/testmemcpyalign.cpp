//          testmemcpyalign.cpp                         Agner Fog 2013-08-07

// Test if memory copying has penalty for false dependence between 
// source and destination addressses
// Timing for different alignments
//
// Instructions: Compile on 64 bit platform and link with testmemcpya.nasm

#include <stdio.h>
#include <memory.h>
#include <string.h>

typedef void * memcpyF (void * dest, const void * src, size_t count); 

extern "C" {
    // function prototypes for CPU specific function versions
    memcpyF testmemcpy0, testmemcpy4, testmemcpy8, testmemcpy16, testmemcpy32;
    void cpuid_ex (int abcd[4], int a, int c);
    unsigned int ReadTSC (void);
}

// Tables of function pointers
const int NUMFUNC = 6;
memcpyF * memcpyTab[NUMFUNC] = {memcpy, testmemcpy0, testmemcpy4, testmemcpy8, testmemcpy16, testmemcpy32};
const char * DispatchNames[NUMFUNC] = {"library", "rep movs", "4 bytes", "8 bytes", "16 bytes", "32 bytes"};

const int kbyte = 1024;
const int Mbyte = kbyte * kbyte;

// Tables of test lengths
const int NUMLENGTHS = 3;
unsigned int lengthTab[NUMLENGTHS] = {4*kbyte, 64*kbyte, 1*Mbyte};


int main () {
    int i, li, os, len;
    int version;
    unsigned int tim, overheadtime;
    memcpyF * func;

    // allocate memory buffer
    const int alignby = 64;         // align by 64
    const int bufsize = 32 * Mbyte;
    char * buffer = new char[bufsize + alignby];
    char * bufa = (char*)((size_t)(buffer + alignby - 1) & -(size_t)alignby); // buffer aligned

    int cpuIdOutput[4];
    // Call cpuid function 1 to see if AVX supported
    cpuid_ex(cpuIdOutput, 1, 0);
    int AVX_supported = (cpuIdOutput[2] >> 28) & 1;

    // measure overhead time
    overheadtime = 99999999;
    for (i=0; i<10; i++) {
        tim = (unsigned int)ReadTSC();
        tim = (unsigned int)ReadTSC() - tim;
        if (tim < overheadtime) overheadtime = tim;
    }

    printf("\nTest memory copying on different alignments");
    printf("\nNumbers are source offset and time");

    // Loop through lengths
    for (li = 0; li < NUMLENGTHS; li++) {
        len = lengthTab[li];
        if (len < kbyte) {        
            printf("\n\nlength %i bytes", len);
        }
        else if (len < Mbyte) {
            printf("\n\n\nlength %i kbytes", len / kbyte);
        }
        else {
            printf("\n\n\nlength %i Mbytes", len / Mbyte);
        }

        // Loop through versions
        for (version = 0; version < NUMFUNC; version++) {

            printf("\n\n%s version", DispatchNames[version]);
            if (version >= 5 && !AVX_supported) {
                printf(" not supported"); continue;
            }

            func = memcpyTab[version];
            for (os = -0x180; os <= 0x60; os += 8) {
                (*func)(bufa, bufa+1*Mbyte+os, len);
                tim = (unsigned int)ReadTSC();
                for (i=0; i<10; i++) {
                    (*func)(bufa, bufa+1*Mbyte+os, len);
                }
                tim = (unsigned int)ReadTSC() - tim;
                printf("\n%4i  %8i", os, tim - overheadtime);
            }
        }
    }
    printf("\n\n\nSearch which modulo produces false dependence");
    len = lengthTab[0];
    printf("\nlength %i kbytes", len / kbyte);

    // Loop through versions
    for (version = 0; version < NUMFUNC; version++) {

        printf("\n\n%s version", DispatchNames[version]);
        if (version >= 5 && !AVX_supported) {
            printf(" not supported"); continue;
        }

        func = memcpyTab[version];

        for (os = 0; os <= 0x4000; os += 0x100) {
            (*func)(bufa, bufa+1*Mbyte+os, len);
            tim = (unsigned int)ReadTSC();
            for (i=0; i<10; i++) {
                (*func)(bufa, bufa+1*Mbyte+os-24, len);
            }
            tim = (unsigned int)ReadTSC() - tim;
            printf("\n0x%04X-0x18  %8i", os, tim - overheadtime);
        }
    }

    // free memory buffer
    delete buffer;
    return 0;
}
