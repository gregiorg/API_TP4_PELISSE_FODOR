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
        struct fb* first_fb;
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

struct fb {
	size_t size;
	struct fb* next;
};

struct ob {
  size_t size;
};

////////////////////////////////////////////////////////////////////////////////

void mem_init(void* mem, size_t taille)
{
  memory_addr = mem;
  *(size_t*)memory_addr = taille;

	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());

  // create the first fb, set it's parametres and place it in the header
  struct fb* new_fb;
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
	struct fb* pt_mem_libre = get_header()->first_fb;


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
			struct ob *struc_occupe = pt_mem_courant;
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




	struct fb *fb=get_header()->fit(/*...*/NULL, /*...*/0);
	/* ... */
	return NULL;
}

struct fb* mem_fit_first(struct fb *list, size_t size) {
	//la fonction doit renvoyer l'adresse du premier bloc libre >= size  dans les blocs libre present dans l'adresse list 

	//return NULL si le bloc n'existe pas
   // sinon renvoyer l'adresse de la struct present dans la liste




	return NULL;
}

struct fb* getPrevious(struct fb* a_pour_previous){
	//on renvoie l'adresse du fb précédent:
		struct fb *pt_mem = get_header()->first_fb;
		//on recupère dans un pointeur la prochaine struc fb
		//si on est dans le cas ou on on demande le previous du premier fb
		//on return null
		if(a_pour_previous == pt_mem){ //pt_mem =  get_header()->first_fb;
			return NULL;
		}

		while (pt_mem < memory_addr+get_header()->memory_size) {
			//tant qu'il y a des adresses de blocs libre
			if( pt_mem->next == a_pour_previous){
				//si le pointeur a pour nextfb celui dont on veut le previous
				return pt_mem;
				//on return ce pointeur
			}else{
				//sinon on avance le pointeur au prochain fb
				pt_mem = pt_mem->next;
			}
		}
		//en cas de situation d'erreur
		printf("Situation d'erreur lors du recherche de la struct fb précdente");
		exit(-1);
		
} 




void mem_free(void* mem) {
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
struct fb* mem_fit_best(struct fb *list, size_t size) {
	return NULL;
}

struct fb* mem_fit_worst(struct fb *list, size_t size) {
	return NULL;
}
