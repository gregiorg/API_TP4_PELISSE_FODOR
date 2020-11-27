/* On inclut l'interface publique */
#include "mem.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

/* Définition de l'alignement recherché
 * Avec gcc, on peut utiliser __BIGGEST_ALIGNMENT__
 * sinon, on utilise 16 qui conviendra aux plateformes qu'on cible
 */
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16
#endif

/* structure placée au début de la zone de l'allocateur

   Elle contient toutes les variables globales nécessaires au
   fonctionnement de l'allocateur

   Elle peut bien évidemment être complétée
*/
struct allocator_header {
        size_t memory_size;
        fb* first_fb;
	      mem_fit_function_t *fit;
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void* memory_addr;

static inline void *get_system_memory_addr() {
	return memory_addr;
}

static inline struct allocator_header *get_header() {
	struct allocator_header *h;
	h = get_system_memory_addr();
	return h;
}

static inline size_t get_system_memory_size() {
	return get_header()->memory_size;
}

////////////////////////////////////////////////////////////////////////////////

typedef struct fb_t {
	size_t size;
	struct fb_t* next;
} fb;

typedef struct ob_t {
  size_t size;
} ob;

////////////////////////////////////////////////////////////////////////////////

void mem_init(void* mem, size_t taille)
{
  memory_addr = mem;
  *(size_t*)memory_addr = taille;

	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());

  // create the first fb, set it's parametres and place it in the header
  fb* new_fb;
  new_fb = memory_addr + sizeof(struct allocator_header);
  new_fb->size = taille;
  new_fb->next = NULL;

  get_header()->first_fb = new_fb;

	mem_fit(&mem_fit_first);
}

void mem_fit(mem_fit_function_t *f) {
	get_header()->fit = f;
}

void mem_show(void (*print)(void *, size_t, int)) {

	//il faut déja un pointeur sur le bloc courant, pointe sur adresse header allocateur memoire + taille du header
 	void *pt_mem_courant = memory_addr + sizeof(struct allocator_header);
	 // pointe vers le prochain bloc de libre (fb sur le schema)
	fb* pt_mem_libre = get_header()->first_fb;


	//tant que l'adresse de fin du bloc global est supérieur au pointeur courant(que l'on deplace de bloc mémoire en bloc mémoire)
	while (pt_mem_courant < memory_addr+get_header()->memory_size) {
		//sur quoi pointe pt_mem_courant:
		//- bloc libre
		//-bloc utilisé
		if( pt_mem_libre != NULL && pt_mem_libre == pt_mem_courant){
			//si c'est le cas, le prochain bloc courant est LIBRE <=> 	pt_mem_courant LIBRE
			//on recpère la taille
			size_t taille_zone_libre = pt_mem_libre->size;
			//affichage d'un bloc LIBRE
			print(pt_mem_courant, taille_zone_libre, 1);
			//on deplace les deux pointeurs aux bon endroits:
			//le pointeur courant vers ke prochain bloc
			//et le pointeur sur la mémoire libre vers le prochain bloc de mémoire libre
			pt_mem_courant +=taille_zone_libre;
			pt_mem_libre = pt_mem_libre->next;
			//c'est comme cela qu'on avance dans notre programme


		}
		else{
			//si on est ici c'est que le prochain bloc est OCCUPE
			//recuperation de la struc pour avoir les differentes données
			//on l'interprete en tant que struct ob (un champ: size_t)
			ob *struc_occupe = pt_mem_courant;
			size_t taille_zone_occuper = struc_occupe->size;

			//affichage d'un bloc OCCUPE
			print(pt_mem_courant, taille_zone_occuper, 0);
			//une fois que c'est affiché, il vaut avancer le pointeur courant
			pt_mem_courant += taille_zone_occuper;
			//pt_mem_courant pointe donc sur la prochaine zone
		}

	}
}



void *mem_alloc(size_t taille) {

	__attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */




	fb* this_fb=get_header()->fit(/*...*/NULL, /*...*/0);
	/* ... */
	return NULL;
}

fb* mem_fit_first(fb *list, size_t size) {
	return NULL;
}

void mem_free(void* mem) {
  ob* current_ob = mem-sizeof(ob); // interpret mem as ob

  // find the last fb before current_ob
  fb* prev_fb = get_header()->first_fb;
  while ((void*) prev_fb->next < (void*) current_ob) {
    prev_fb = prev_fb->next;
  } // prev_fb->next >= current_ob

  fb* next_fb = prev_fb->next; // find the first fb after current_ob;

  /* /!\ prev_fb and next_fb can both be null /!\
     if we're at the begining or the end of the total memory zone
     however, it shoudn't change the algorithme. it just needs to be detected */

  // these booleean will help us determine the free scenario (cf. free_mem.drawio)
  int is_previous_fb = prev_fb != NULL && (void*) (prev_fb + prev_fb->size) == (void*) current_ob;
  int is_next_fb = next_fb != NULL && (void*) (current_ob + current_ob->size) == (void*) next_fb;

  if (is_previous_fb && !is_next_fb) {

  } else if (!is_previous_fb && is_next_fb) {

  } else if (!is_previous_fb && ! is_next_fb) {

  } else/*(is_previous_fb && is_next_fb)*/{

  }
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone) {
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return 0;
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
fb* mem_fit_best(fb *list, size_t size) {
	return NULL;
}

fb* mem_fit_worst(fb *list, size_t size) {
	return NULL;
}
