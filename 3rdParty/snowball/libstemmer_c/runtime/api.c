
#include <stdlib.h> /* for calloc, free */
#include "header.h"

#if 0 //DEPRECATED
extern struct SN_env * SN_create_env(int S_size, int I_size, int B_size)
{
    struct SN_env * z = (struct SN_env *) calloc(1, sizeof(struct SN_env));
    if (z == NULL) return NULL;
    z->p = create_s();
    if (z->p == NULL) goto error;
    if (S_size)
    {
        int i;
        z->S = (symbol * *) calloc(S_size, sizeof(symbol *));
        if (z->S == NULL) goto error;

        for (i = 0; i < S_size; i++)
        {
            z->S[i] = create_s();
            if (z->S[i] == NULL) goto error;
        }
    }

    if (I_size)
    {
        z->I = (int *) calloc(I_size, sizeof(int));
        if (z->I == NULL) goto error;
    }

    if (B_size)
    {
        z->B = (unsigned char *) calloc(B_size, sizeof(unsigned char));
        if (z->B == NULL) goto error;
    }

    return z;
error:
    SN_close_env(z, S_size);
    return NULL;
}
#endif

extern int SN_init_env( struct SN_env* env, int S_size, int I_size, int B_size)
{
	std::memset( env, 0, sizeof(*env));
	if (S_size > max_S_size || I_size > max_I_size || B_size > max_B_size) return -1;
	env->S = buf_S;
	env->I = buf_I;
	env->B = buf_B;
	int si=0;
	for (; si <= S_size; ++si)
	{
		env->symbuf_S[ si].capacity = SymbolBufSize;
	}
	sev->p = env->symbuf_S[ 0].buf; 
	si = 1;
	for (; si < S_size; ++si)
	{
		env->S[ si-1] = env->symbuf_S[ si].buf;
	}
}


