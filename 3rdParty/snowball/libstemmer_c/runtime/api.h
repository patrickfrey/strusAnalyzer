#ifndef _SNOWBALL_API
#define _SNOWBALL_API

typedef unsigned char symbol;

#define SymbolBufSize 120

typedef struct
{
	int capacity;
	int size;
	unsigned char buf[ SymbolBufSize];
} symbol_struct;

/* Or replace 'char' above with 'short' for 16 bit characters.

   More precisely, replace 'char' with whatever type guarantees the
   character width you need. Note however that sizeof(symbol) should divide
   HEAD, defined in header.h as 2*sizeof(int), without remainder, otherwise
   there is an alignment problem. In the unlikely event of a problem here,
   consult Martin Porter.

*/

#define max_S_size 4
#define max_I_size 4
#define max_B_size 4

struct SN_env {
    symbol * p;
    int c; int l; int lb; int bra; int ket;
    symbol * * S;
    int * I;
    unsigned char * B;

    symbol* buf_S[ max_S_size];
    int buf_I[ max_I_size];
    unsigned char buf_B[ max_B_size];
    symbol_struct symbuf_S[ max_S_size];
};

extern int SN_init_env( struct SN_env* env, int S_size, int I_size, int B_size);

extern int SN_set_current(struct SN_env * z, int size, const symbol * s);

#endif
