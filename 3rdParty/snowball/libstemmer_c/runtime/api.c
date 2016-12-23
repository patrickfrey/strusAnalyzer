
#include <string.h> /* for memset */
#include "header.h"

extern int SN_init_env( struct SN_env* env, int S_size, int I_size, int B_size)
{
	memset( env, 0, sizeof(*env));
	if (S_size > max_S_size || I_size > max_I_size || B_size > max_B_size) return -1;
	env->S = env->buf_S;
	env->I = env->buf_I;
	env->B = env->buf_B;
	int si=0;
	for (; si <= S_size; ++si)
	{
		env->symbuf_S[ si].capacity = SymbolBufSize;
	}
	env->p = env->symbuf_S[ 0].buf; 
	si = 1;
	for (; si < S_size; ++si)
	{
		env->S[ si-1] = env->symbuf_S[ si].buf;
	}
	return 0;
}

extern int SN_set_current(struct SN_env * z, int size, const symbol * s)
{
    int err = replace_s(z, 0, z->l, size, s, NULL);
    z->c = 0;
    return err;
}

