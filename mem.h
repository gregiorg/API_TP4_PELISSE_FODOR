#ifndef __MEM_H
#define __MEM_H
#include <stddef.h>

typedef struct fb_t fb;
struct ob;

/* fonctions principales de l'allocateur */
void mem_init(void* mem, size_t taille);
void* mem_alloc(size_t size);
void mem_free(void *ptr);
void* mem_realloc(void *old, size_t new_size);

/* Itération sur le contenu de l'allocateur */
/* nécessaire pour le mem_shell */
void mem_show(void (*print)(void *adr, size_t size, int free));

/* Choix de la stratégie et strategies usuelles */
/* Si vous avez le temps... */
typedef struct fb_t* (mem_fit_function_t)(fb*, size_t);

void mem_fit(mem_fit_function_t*);
mem_fit_function_t mem_fit_first;
mem_fit_function_t mem_fit_worst;
mem_fit_function_t mem_fit_best;

//fonctions ajoutés par nous-mêmes, nous sommes persuadés qu'elle sont utiles
fb* getPrevious(fb* a_pour_previous);

#endif
