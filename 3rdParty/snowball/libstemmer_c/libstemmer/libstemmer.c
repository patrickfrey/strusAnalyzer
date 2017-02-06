
#include <stdlib.h>
#include <string.h>
#include "../include/libstemmer.h"
#include "../runtime/api.h"
#include "modules_utf8.h"

struct sb_stemmer {
    int (*initenv)( struct SN_env*);
    int (*stem)(struct SN_env *);
};

extern const char **
sb_stemmer_list(void)
{
    return algorithm_names;
}

static stemmer_encoding_t
sb_getenc(const char * charenc)
{
    struct stemmer_encoding * encoding;
    if (charenc == NULL) return ENC_UTF_8;
    for (encoding = encodings; encoding->name != 0; encoding++) {
	if (strcmp(encoding->name, charenc) == 0) break;
    }
    if (encoding->name == NULL) return ENC_UNKNOWN;
    return encoding->enc;
}

static struct sb_stemmer* stemmer_new_without_state(const char * algorithm, const char * charenc)
{
	stemmer_encoding_t enc;
        struct stemmer_modules * module;
        struct sb_stemmer * stemmer;
    
        enc = sb_getenc(charenc);
        if (enc == ENC_UNKNOWN) return NULL;
    
        for (module = modules; module->name != 0; module++) {
	    if (strcmp(module->name, algorithm) == 0 && module->enc == enc) break;
        }
        if (module->name == NULL) return NULL;
        
        stemmer = (struct sb_stemmer *) malloc(sizeof(struct sb_stemmer));
        if (stemmer == NULL) return NULL;
    
        stemmer->initenv = module->initenv;
        stemmer->stem = module->stem;
        return stemmer;
}

extern struct sb_stemmer *
sb_stemmer_new(const char * algorithm, const char * charenc)
{
    struct sb_stemmer *stemmer = stemmer_new_without_state( algorithm, charenc);
    return stemmer;
}

extern struct sb_stemmer *
sb_stemmer_new_threadsafe( const char * algorithm, const char * charenc)
{
    struct sb_stemmer *stemmer = stemmer_new_without_state( algorithm, charenc);
    return stemmer;
}

extern int sb_stemmer_UTF_8_init_env( const struct sb_stemmer * stemmer, sb_stemmer_env* env)
{
	return stemmer->initenv( (struct SN_env*)(void*)env);
}

void
sb_stemmer_delete(struct sb_stemmer * stemmer)
{
    if (stemmer == NULL) return;
    free( stemmer);
}

const sb_symbol *
sb_stemmer_stem_threadsafe( const struct sb_stemmer * stemmer, sb_stemmer_env* env_, const sb_symbol * word, int size)
{
    int ret;
    struct SN_env* env = (struct SN_env*)(void*)env_;
    if (SN_set_current( env, size, (const symbol *)(word)))
    {
        env->l = 0;
        return NULL;
    }
    ret = stemmer->stem( env);
    if (ret < 0) return NULL;
    env->p[ env->l] = 0;
    return (const sb_symbol *)( env->p);
}

int
sb_stemmer_length_threadsafe( sb_stemmer_env* env_)
{
    struct SN_env* env = (struct SN_env*)(void*)env_;
    return env->l;
}
