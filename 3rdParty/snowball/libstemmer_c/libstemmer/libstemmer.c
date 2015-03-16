
#include <stdlib.h>
#include <string.h>
#include "../include/libstemmer.h"
#include "../runtime/api.h"
#include "modules.h"

struct sb_stemmer {
    struct SN_env * (*create)(void);
    void (*close)(struct SN_env *);
    int (*stem)(struct SN_env *);

    struct SN_env * env;
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
    
        stemmer->create = module->create;
        stemmer->close = module->close;
        stemmer->stem = module->stem;
        stemmer->env = NULL;
        return stemmer;
}

extern struct sb_stemmer *
sb_stemmer_new(const char * algorithm, const char * charenc)
{
    struct sb_stemmer *stemmer = stemmer_new_without_state( algorithm, charenc);
    stemmer->env = stemmer->create();
    if (stemmer->env == NULL)
    {
        sb_stemmer_delete(stemmer);
        return NULL;
    }
    return stemmer;
}

extern struct sb_stemmer *
sb_stemmer_new_threadsafe( const char * algorithm, const char * charenc)
{
    struct sb_stemmer *stemmer = stemmer_new_without_state( algorithm, charenc);
    return stemmer;
}

extern struct SN_env*
sb_stemmer_create_env( const struct sb_stemmer * stemmer)
{
	return stemmer->create();
}

void
sb_stemmer_delete_env( const struct sb_stemmer * stemmer, struct SN_env* env)
{
	if (env) stemmer->close( env);
}

void
sb_stemmer_delete(struct sb_stemmer * stemmer)
{
    if (stemmer == 0) return;
    if (stemmer->close == 0) return;
    if (stemmer->env) stemmer->close(stemmer->env);
    stemmer->close = 0;
    free(stemmer);
}

const sb_symbol *
sb_stemmer_stem(struct sb_stemmer * stemmer, const sb_symbol * word, int size)
{
    int ret;
    if (SN_set_current(stemmer->env, size, (const symbol *)(word)))
    {
        stemmer->env->l = 0;
        return NULL;
    }
    ret = stemmer->stem(stemmer->env);
    if (ret < 0) return NULL;
    stemmer->env->p[stemmer->env->l] = 0;
    return (const sb_symbol *)(stemmer->env->p);
}

const sb_symbol *
sb_stemmer_stem_threadsafe( const struct sb_stemmer * stemmer, struct SN_env* env, const sb_symbol * word, int size)
{
    int ret;
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
sb_stemmer_length(struct sb_stemmer * stemmer)
{
    return stemmer->env->l;
}

int
sb_stemmer_length_threadsafe( struct SN_env* env)
{
    return env->l;
}
