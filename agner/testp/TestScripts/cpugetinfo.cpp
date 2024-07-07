//                  cpugetinfo.cpp
//
// Get cpuid info:
// vendor, family, model, cache parameters, instruction set
//
// Instructions:
// Call with zero or one command line parateters:
// Command line parameter:
// vendor:          Prints CPU vendor (Intel, AMD, VIA)
// brand:           Prints CPU vendor brand string
// family:          Prints CPU family number. add " hex" for hexadecimal
// model:           Prints CPU model number. add " hex" for hexadecimal
// stepping:        Prints CPU stepping. add " hex" for hexadecimal
// cache1size:      Prints size of level 1 data cache
// cache2size:      Prints size of level 2 data cache
// cache3size:      Prints size of level 3 data cache
// cachesize:       Prints size of last level cache
// cachelinesize:   Prints line size of level 1 data cache
// system:          Prints UNIX or WINDOWS
// instructionsets: Prints a list of instruction sets supported by the CPU
// name of an instruction set (e.g. SSE3): 1 if supported by CPU, 0 if not
// help:            Prints instructions
// all:
// Prints vendor; family, model and stepping (as hexadecimal numbers);
// Extended vendor string, cache sizes, instruction sets supported by CPU.
// (does not check if instruction sets are supported by operating system)
//
// Compile instructions:
// Compile for console mode, no unicode, any x86 platform.
// Remember to update the InstrSet table with any new instruction sets.
//
// Last modified: 2022-11-04
// (c) 2013-2022 GNU General Public License www.gnu.org/licenses
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

// function prototypes
void getCacheSize(bool printout);
void printInstructionSets();
int  checkInstructionSet(const char * iname);
void printHelp();

// global variables
int L1Size = 0;
int L2Size = 0;
long long int L3Size = 0;
int LineSize = 0;


//////////////////////////////////////////////////////////////////////////////
//              Table of instruction set descriptors                        //
//                                                                          //
// You may extend this table with any new instruction sets.                 //
// Each entry contains:                                                     //
// leaf:   value of eax for cpuid input (ecx = 0)                           //
// regist: cpuid output register (B = ebx, C = ecx, D = edx)                //
// bit:    bit number in output register                                    //
// name:   name of instruction set                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

struct InstrSet {
    unsigned int leaf;     // eax input to cpuid
    int ecx;               // ecx input to cpuid
    int regist;            // cpuid output register
    int bit;               // bit number in cpuid output register
    const char * name;     // name of instruction set
};

// Indicate register eax, ebx, ecx, edx, respectively, in isets table
const int A = 0, B = 1, C = 2, D = 3;

static const InstrSet isets[] = {
    {1, 0, D,  0, "X87"},
    {1, 0, D,  4, "TSC"},
    {1, 0, D,  5, "MSR"},
    {1, 0, D,  8, "CX8"},
    {1, 0, D, 15, "CMOV"},
    {1, 0, D, 18, "PSN"},
    {1, 0, D, 19, "CLFSH"},
    {1, 0, D, 23, "MMX"},
    {1, 0, D, 24, "FXSR"},
    {1, 0, D, 25, "SSE"},
    {1, 0, D, 26, "SSE2"},
    {1, 0, D, 30, "IA64"},
    {1, 0, C,  0, "SSE3"},
    {1, 0, C,  1, "PCLMULQDQ"},
    {1, 0, C,  3, "MONITOR"},
    {1, 0, C,  9, "SSSE3"},
    {1, 0, C, 12, "FMA"},
    {1, 0, C, 12, "FMA3"},
    {1, 0, C, 13, "CMPXCHG16B"},
    {1, 0, C, 19, "SSE4.1"},
    {1, 0, C, 20, "SSE4.2"},
    {1, 0, C, 22, "MOVBE"},
    {1, 0, C, 23, "POPCNT"},
    {1, 0, C, 25, "AES"},
    {1, 0, C, 26, "XSAVE"},
    {1, 0, C, 27, "OSXSAVE"},
    {1, 0, C, 28, "AVX"},
    {1, 0, C, 29, "F16C"},
    {1, 0, C, 30, "RDRAND"},

    {7, 1, A,  5, "AVX512BF16"},
    {7, 0, B,  5, "AVX2"},
    {7, 0, B,  3, "BMI1"},
    {7, 0, B,  4, "HLE"},
    {7, 0, B,  7, "SMEP"},
    {7, 0, B,  8, "BMI2"},
    {7, 0, B,  9, "ERMS"},
    {7, 0, B, 10, "INVPCID"},
    {7, 0, B, 11, "RTM"},
    {7, 0, B, 14, "MPX"},
    {7, 0, B, 16, "AVX512"},
    {7, 0, B, 16, "AVX512F"},
    {7, 0, B, 17, "AVX512DQ"},
    {7, 0, B, 18, "RDSEED"},
    {7, 0, B, 19, "ADX"},
    {7, 0, B, 20, "SMAP"},
    {7, 0, B, 21, "AVX512IFMA"},
    {7, 0, B, 23, "CLFLUSHOPT"},
    {7, 0, B, 24, "CLWB"},
    {7, 0, B, 25, "IntelPT"},
    {7, 0, B, 26, "AVX512PF"},
    {7, 0, B, 27, "AVX512ER"},
    {7, 0, B, 28, "AVX512CD"},
    {7, 0, B, 29, "SHA"},
    {7, 0, B, 30, "AVX512BW"},
    {7, 0, B, 31, "AVX512VL"},
    {7, 0, C,  0, "PREFETCHWT1"},    
    {7, 0, C,  1, "AVX512VBMI"},    
    {7, 0, C,  6, "AVX512VBMI2"},
    {7, 0, C,  8, "GFNI"},
    {7, 0, C,  9, "VAES"},
    {7, 0, C, 10, "VPCLMULQDQ"},
    {7, 0, C, 11, "AVX512VNNI"},
    {7, 0, C, 12, "AVX512BITALG"},
    {7, 0, C, 14, "AVX512VPOPCNTDQ"},
    {7, 0, C, 27, "MOVDIRI"},
    {7, 0, C, 28, "MOVDIR64B"},
    {7, 0, D,  2, "AVX512_4VNNIW"},
    {7, 0, D,  3, "AVX512_4FMAPS"},
    {7, 0, D,  4, "Fast_Short_REP_MOV"},
    {7, 0, D,  8, "AVX512_VP2INTERSECT"},
    {7, 0, D,  10, "MD_CLEAR"},
    {7, 0, D,  15, "HYBRID_CPU"},
    {7, 0, D,  20, "CET_IBT"},
    {7, 0, D,  23, "AVX512_FP16"},

    {0xD, 0, A,  0, "XSAVEOPT"},
    {0xD, 0, A,  1, "XSAVEC"},
    {0xD, 1, A,  3, "XSAVES"},  // ecx=1

    // AMD instructions, and instructions first implemented by AMD:
    {0x80000001, 0, C,  0, "LAHF_64"},
    {0x80000001, 0, C,  5, "LZCNT"},
    {0x80000001, 0, C,  6, "SSE4A"},
    {0x80000001, 0, C,  8, "PREFETCHW"},
    {0x80000001, 0, C, 11, "XOP"},
    {0x80000001, 0, C, 16, "FMA4"},
    {0x80000001, 0, C, 21, "TBM"},
    {0x80000001, 0, D, 22, "MMXEXT"},
    {0x80000001, 0, D, 27, "RDTSCP"},
    {0x80000001, 0, D, 30, "3DNOWEXT"},
    {0x80000001, 0, D, 31, "3DNOW"},
    {0x80000008, 0, B,  0, "CLZERO"},
    {0x80000008, 0, B,  4, "RDPRU"},
    {0x80000008, 0, B,  8, "MCOMMIT"},
    {0x80000008, 0, B,  9, "WBNOINVD"},
    {0x80000008, 0, B,  8, "MCOMMIT"},
    {0x8000001F, 0, A,  4, "SNP"},

    // VIA instructions:
    {0xC0000001, 0, D,  2, "XSTORE"},
    {0xC0000001, 0, D,  3, "XSTORE_ENABLED"},
    {0xC0000001, 0, D,  6, "XCRYPT"},
    {0xC0000001, 0, D,  7, "XCRYPT_ENABLED"},
    {0xC0000001, 0, D,  8, "ACE2"},
    {0xC0000001, 0, D,  9, "ACE2_ENABLED"},
    {0xC0000001, 0, D, 10, "XSHA"},
    {0xC0000001, 0, D, 11, "XSHA_ENABLED"},
    {0xC0000001, 0, D, 12, "MONTMUL"},
    {0xC0000001, 0, D, 13, "MONTMUL_ENABLED"}
};

const int isets_len = sizeof(isets) / sizeof(*isets);


#ifdef _MSC_VER
#define strcasecmp strcasecmp
#endif

//////////////////////////////////////////////////////////////////////////////
//                      cpuid function
//////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__  // Gnu compiler

// define cpuid with inline assembly
inline static void cpuid (unsigned int output[4], unsigned int aa, unsigned int cc = 0) {	
    int a, b, c, d;
    __asm("cpuid" : "=a"(a),"=b"(b),"=c"(c),"=d"(d) : "a"(aa),"c"(cc) : );
    output[0] = a;
    output[1] = b;
    output[2] = c;
    output[3] = d;
}

#elif defined (_MSC_VER)  // Microsoft compiler

#include "intrin1.h"  // define intrinsic function __cpuidex
inline static void cpuid (unsigned int output[4], unsigned int aa, unsigned int cc = 0) {	
    __cpuidex((int*)output, aa, cc);
}

#else  // Other compiler. Use inline assembly

#ifdef __x86_64__  // 64 bit mode

inline static void cpuid (unsigned int output[4], unsigned int aa, unsigned int cc = 0) {	
    __asm {
        mov r8, output;
        mov eax, aa;
        mov ecx, cc;
        cpuid;
        mov [r8],    eax;
        mov [r8+4],  ebx;
        mov [r8+8],  ecx;
        mov [r8+12], edx;
    }
}

#else  // 32 bit mode

inline static void cpuid (unsigned int output[4], unsigned int aa, unsigned int cc = 0) {	
    __asm {
        mov eax, aa;
        mov ecx, cc;
        cpuid;
        mov esi, output;
        mov [esi],    eax;
        mov [esi+4],  ebx;
        mov [esi+8],  ecx;
        mov [esi+12], edx;
    }
}

#endif
#endif


//////////////////////////////////////////////////////////////////////////////
//                      main function
//////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[]) {

    unsigned int i;
    unsigned int cpuIdOutput[4];

    // Call cpuid function 0
    cpuid(cpuIdOutput, 0);

    // vendor
    union {
        char text[64];
        unsigned int id[16];
    } str;
    str.id[0] = cpuIdOutput[1];
    str.id[1] = cpuIdOutput[3];
    str.id[2] = cpuIdOutput[2];
    str.id[3] = 0;

    if (argc <= 1 || strstr(argv[1], "help")) {
        printHelp();
        return 1;
    }

    if (strstr(argv[1], "vendor")) {
        // print vendor only
        if (strstr(str.text, "Intel")) printf("Intel");
        else if (strstr(str.text, "AMD")) printf("AMD");
        else if (strstr(str.text, "Centaur") || strstr(str.text, "VIA")) printf("VIA");
        else printf("%s", str.text);
        return 0;  // do nothing else
    }
    if (strstr(argv[1], "all") || strstr(argv[1], "brand")) {
        printf("\nvendor %s", str.text); 

        // get extended description
        cpuid(cpuIdOutput, 0x80000000);
        if (cpuIdOutput[0] >= 0x80000004) {
            // get extended vendor string
            unsigned int * t = str.id;
            for (i = 0x80000002; i <= 0x80000004; i++) {
                cpuid(cpuIdOutput, i);
                *t++ = cpuIdOutput[A];
                *t++ = cpuIdOutput[B];
                *t++ = cpuIdOutput[C];
                *t++ = cpuIdOutput[D];
            }
            *t = 0;  // terminate string
            // print string
            printf("\n %s\n", str.text);
        }
    }

    // get family etc
    // Call cpuid function 1
    cpuid(cpuIdOutput, 1);
    int family = ((cpuIdOutput[0] >> 8) & 0x0F) + ((cpuIdOutput[0] >> 20) & 0xFF);   // family code
    int model  = ((cpuIdOutput[0] >> 4) & 0x0F) | ((cpuIdOutput[0] >> 12) & 0xF0);   // model code
    int stepping = cpuIdOutput[0] & 0x0F;
    const char * numberformat = 0;
    if (argc > 2 && strstr(argv[2], "hex")) {
        numberformat = "0x%02X";    // hexadecimal
    }
    else {
        numberformat = "%i";       // decimal
    }
    if (strstr(argv[1], "family")) {        
        // print family number only, decimal
        printf(numberformat, family);  
        return 0;  // do nothing else
    }
    if (strstr(argv[1], "model")) {        
        // print model number only, decimal
        printf(numberformat, model);  
        return 0;  // do nothing else
    }
    if (strstr(argv[1], "stepping")) {        
        // print stepping number only, decimal
        printf(numberformat, stepping);  
        return 0;  // do nothing else
    }
    if (strstr(argv[1], "instructionset")) {        
        printInstructionSets();
        return 0;
    }
    if (strstr(argv[1], "system")) {
#if defined (__WINDOWS__) || defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        printf("WINDOWS");
#elif defined (__unix__) || defined (__linux__)
        printf("UNIX");
#else
        printf("UNKNOWN");
#endif
        return 0;
    }
    if (strstr(argv[1], "cache1size")) {
        // Prints size of level 1 data cache
        getCacheSize(false);
        printf("%i", L1Size);  
        return 0;
    }
    if (strstr(argv[1], "cache2size")) {
        // Prints size of level 2 data cache
        getCacheSize(false);
        printf("%i", L2Size);  
        return 0;
    }
    if (strstr(argv[1], "cache3size")) {
        // Prints size of level 3 data cache
        getCacheSize(false);
        printf("%lli", L3Size);  
        return 0;
    }
    if (strstr(argv[1], "cachesize")) {
        // Prints size of last level cache
        getCacheSize(false);
        if (L3Size) {
            printf("%lli", L3Size);
        }
        else if (L2Size) {
            printf("%i", L2Size);
        }
        else {
            printf("%i", L1Size);
        }
        return 0;
    }
    if (strstr(argv[1], "cachelinesize")) {
        // Prints line size of level 1 data cache
        getCacheSize(false);
        printf("%i", LineSize);  
        return 0;
    }
    if (strstr(argv[1], "help") || strstr(argv[1], "?")) {        
        // print help text only
        printHelp();  
        return 0;  // do nothing else
    }
    if (strstr(argv[1], "brand")) {
        return 0;
    }
    if (strstr(argv[1], "all")) {
        // print everything 
        printf("\nFamily 0x%X, model 0x%X, stepping 0x%X\n", family, model, stepping);
        //printf("\nFeatures ecx 0x%08X\nFeatures edx 0x%08X", cpuIdOutput[2], cpuIdOutput[3]);
        getCacheSize(true);
        printf("\n\nInstruction sets supported by CPU:\n");
        printInstructionSets();
        printf("\n");
        return 0;
    }

    // None of these. Command is an instruction set
    // print support for instruction set
    char * iname = argv[1], * nameend, *p;
    // skip leading whitespace
    while ((*iname <= ' ' || *iname == '-' || *iname == '/') && *iname) iname++;
    // remove trailing whitespace
    nameend = iname + strlen(iname) - 1;
    while (*nameend <= ' ' && *nameend) *nameend = 0;
    // replace '_' by '.'
    for (p = iname; *p; p++) {
        if (*p == '_') *p = '.';
    }
    // check if supported
    int supported = checkInstructionSet(iname);
    if (supported < 0) {
        printf("\nunknown name %s\n", iname);
        return 1;
    }
    // write result and return
    printf("%i", supported);
    return 0; 
};


//////////////////////////////////////////////////////////////////////////////
//                      getCacheSize function
//////////////////////////////////////////////////////////////////////////////

void getCacheSize(bool printout) {
    unsigned int cpuIdOutput[4];

    // Call cpuid function 2
    cpuid(cpuIdOutput, 2);
    if (printout) {
        printf("\nCache descriptors:\n  0x%08X\n  0x%08X\n  0x%08X\n  0x%08X", 
            cpuIdOutput[0], cpuIdOutput[1], cpuIdOutput[2], cpuIdOutput[3]);
    }

    // Get cache size by AMD/VIA method: cpuid function 0x80000005
    cpuid(cpuIdOutput, 0x80000000);
    if (cpuIdOutput[0] >= 0x80000005) {
        // Use AMD method for cache size
        cpuid(cpuIdOutput, 0x80000005);
        unsigned int t, a, b, c, size;
        t = cpuIdOutput[3];
        if (t && printout) {
            printf("\n\nL1 Instruction cache: size %i kB, ways %i, lines/tag %i, line size %i",
                (t>>24)&0xFF, (t>>16)&0xFF, (t>>8)&0xFF, t&0xFF);
        }
        t = cpuIdOutput[2];
        L1Size = ((t>>24) & 0xFF) << 10;
        LineSize = t & 0xFF;
        if (t && printout) {
            printf("\n\nL1 Data cache: size %i kB, ways %i, lines/tag %i, line size %i",
                (t>>24)&0xFF, (t>>16)&0xFF, (t>>8)&0xFF, t&0xFF);
        }
        cpuid(cpuIdOutput, 0x80000006);
        // calculate associativity L2
        t = cpuIdOutput[2];
        if (t) {
            size = (t>>16)&0xFFFF;
            L2Size = size << 10;
            a = (t>>12) & 0x0F;  b = 1 << (a/2);
            if (a & 1) b += b/2;
            if (printout) {
                printf("\n\nL2 cache: size %i kB, ways %i, lines/tag %i, line size %i",
                    size, b, (t>>8)&0x0F, t&0xFF);
                if (size & (size - 1)) {
                    // not a power of 2. Inaccurate for Intel processors
                    printf(" (may be inaccurate)");
                }
            }
        }
        // calculate associativity L3
        t = cpuIdOutput[3];
        if (t) {
            a = (t>>12) & 0x0F;  b = 1 << (a/2);
            if (a & 1) b += b/2;
            // size L3
            c = (t>>18)&0x3FFF;
            size = c << 9;
            L3Size = (long long int)size << 10;
            if (printout) {
                if (c & 1) {        
                    printf("\n\nL3 cache: size %i kB, ways %i, lines/tag %i, line size %i",
                        c << 9, b, (t>>8)&0x0F, t&0xFF);
                }
                else {
                    printf("\n\nL3 cache: size %i MB, ways %i, lines/tag %i, line size %i",
                        c >> 1, b, (t>>8)&0x0F, t&0xFF);
                }
                if (size & (size - 1)) {
                    // not a power of 2. Inaccurate for Intel processors
                    printf(" (may be inaccurate)");
                }
            }
        }
    }


    // Get cache size by Intel method: cpuid function 4
    unsigned int subleaf = 0;  cpuIdOutput[0] = 0;
    for (subleaf = 0; subleaf < 20; subleaf++) {    
        cpuid(cpuIdOutput, 4, subleaf);
        int type = cpuIdOutput[0] & 0x1F;
        if (!type) break;
        if (printout) {
            switch (type) {
            case 1:
                printf("\n\nData cache"); break;
            case 2:
                printf("\n\nInstruction cache"); break;
            case 3:
                printf("\n\nUnified cache"); break;
            default:
                printf("\n\nUnknown type %i",type); break;
            }
        }
        int level = ((cpuIdOutput[0] >> 5) & 7);
        int associative = ((cpuIdOutput[0] >> 9) & 1);
        int logicalproc = ((cpuIdOutput[0] >> 14) & 0xFFF) + 1;
        int cores = ((cpuIdOutput[0] >> 26) & 0x3F) + 1;

        int linesize = ((cpuIdOutput[1] >> 0) & 0xFFF) + 1;
        int partitions = ((cpuIdOutput[1] >> 12) & 0x3FF) + 1;
        int ways = ((cpuIdOutput[1] >> 22) & 0x3FF) + 1;
        int sets = (cpuIdOutput[2] >> 0) + 1;
        int size = ways * partitions * linesize * sets;

        if ((type == 1 || type == 3) && size) {
            switch (level) {
            case 1:
                L1Size = size;
                LineSize = linesize;
                break;
            case 2:
                L2Size = size;
                break;
            case 3:
                L3Size = ways * partitions;
                L3Size *= linesize * sets;
                break;
            }
        }
        if (printout) {
            printf(" level %i, ", level);
            const int kb = 1024, mb = kb*kb;
            if (size < kb || size % kb != 0) {
                printf("size %i Bytes", size);
            }
            else if (size < mb || size % mb != 0) {
                printf("size %i kB", size / kb);
            }
            else {
                printf("size %i MB", size / mb);
            }
            printf("\nways %i, partitions %i, linesize %i, sets %i",
                ways, partitions, linesize, sets);
            printf("\nassociative %s, logical proc %i, cores %i",
                (associative ? "yes" : "no"), logicalproc, cores);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//                      printInstructionSets function
//////////////////////////////////////////////////////////////////////////////

// Print all supported instruction sets
void printInstructionSets() {
    unsigned int i;
    unsigned int leaf;
    unsigned int lastleaf = 0;
    unsigned int subleaf;
    unsigned int lastsubleaf = 0;
    unsigned int reg;
    unsigned int maxleaf = 0;
    unsigned int cpuIdOutput[4];

    // loop through isets table
    for (i = 0; i < isets_len; i++) {
        leaf = isets[i].leaf;
        subleaf = isets[i].ecx;
        if (leaf != lastleaf || subleaf != lastsubleaf ) {
            lastleaf = leaf;  lastsubleaf = subleaf;
            // check if leaf supported
            cpuid(cpuIdOutput, leaf & 0xFF000000);
            maxleaf = cpuIdOutput[0];
            if (leaf <= maxleaf) {
                // read leaf
                cpuid(cpuIdOutput, leaf, subleaf);
            }
            else { 
                // not supported
                cpuIdOutput[A] = cpuIdOutput[B] = cpuIdOutput[C] = cpuIdOutput[D] = 0;
            }
        }
        reg = isets[i].regist;
        if (reg > 3) continue;  // skip illegal value
        if (cpuIdOutput[reg] & (1 << isets[i].bit)) {
            // instruction set supported
            printf(" %s", isets[i].name);
        }
    }
    printf(" \n");
}

//////////////////////////////////////////////////////////////////////////////
//                      checkInstructionSet function
//////////////////////////////////////////////////////////////////////////////

// Check if instruction set iname is supported
// Return value:
//  0: not supported
//  1: supported
// -1: unknown iname

int checkInstructionSet(const char * iname) {
    int i;
    int leaf;
    int bit;
    unsigned int reg;
    int maxleaf;
    unsigned int cpuIdOutput[4];

    // loop through isets table
    for (i = 0; i < isets_len; i++) {
        //        if (strcasecmp(isets[i].name, iname) == 0) {
        if (strcasecmp(isets[i].name, iname) == 0) {
            // found in table
            leaf = isets[i].leaf;
            reg  = isets[i].regist;
            bit  = isets[i].bit;
            // check if leaf supported
            cpuid(cpuIdOutput, leaf & 0xFF000000);
            maxleaf = cpuIdOutput[0];
            if (leaf > maxleaf || reg > 3) {
                // not supported
                return 0;
            }
            // read leaf
            cpuid(cpuIdOutput, leaf);
            // check bit and return value
            return (cpuIdOutput[reg] >> bit) & 1;
        }
    }
    // iname not found in table
    return -1;
}

//////////////////////////////////////////////////////////////////////////////
//                      printHelp function
//////////////////////////////////////////////////////////////////////////////

// print help text
void printHelp() {
    printf("\nInstructions:");
    printf("\nCall with one of these command line parateters:");
    printf("\nvendor:    Prints CPU vendor (Intel, AMD, VIA)");
    printf("\nbrand:     Prints CPU vendor extended brand string");
    printf("\nfamily:    Prints CPU family number. \"family hex\" gives hexadecimal number");
    printf("\nmodel:     Prints CPU model number. \"model hex\" gives hexadecimal number");
    printf("\nstepping:  Prints CPU stepping. \"steppint hex\" gives hexadecimal number");
    printf("\nsystem:    Prints UNIX or WINDOWS");
    printf("\ninstructionsets: Prints a list of instruction sets supported by the CPU");
    printf("\nname of an instruction set: 1 if supported by CPU, 0 if not");
    printf("\nhelp:      Prints these instructions");
    printf("\nAll:");
    printf("\nPrints vendor, family, model and stepping (as hexadecimal numbers),");
    printf("\nextended vendor string, cache parameters, instruction sets supported by CPU");
    printf("\n(does not check if instruction sets are supported by operating system).\n");
}
