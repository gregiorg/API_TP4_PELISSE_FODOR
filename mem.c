/* On inclut l'interface publique */
#include "mem.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

	mem_fit(&mem_fit_first); //modifier cette endroit si on veut laisser 
	//la personne choisir quel mem_fit utiliser
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
	//size = taille de la zone a allouer
	//retourne pointeur vers zone allouée et null sinon
	__attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */
	//on parcourt tous les espaces vides jusqu'a avoir un espaces assez grand:
	//la fonction mem_fit_first renvoyer un pointeur sur premier bloc libre
	fb* pt_zone = mem_fit_first(get_header()->first_fb, taille);
	//renvoie un pointeur sur la prochiane zone libre
	//get_header()->first_fb recupère l'adresse de la fb suivante (fb = zone mémoire libre) taille = taille que l'on désire 
	//si on ne peut pas allouer on renvoie null
	if(pt_zone == NULL){
		return pt_zone;
	}
	//sinon plusieurs cas possibles:
	//--> on alloue et il ne reste aucune place mémoire après
	else if(pt_zone->size - taille <= sizeof(fb) && (pt_zone - taille >=0)){
		//si le bloc libre - la taille qu'on désire est inférieur à la taille d'une nouvelle zone libre --> toute la mémoire est libre
		
		fb* pt_to_save = pt_zone->next;
		//tout d'abord, on mets de coté l'adresse du next du bloc fb qui sera transformé en ob


		fb* previous = getPrevious(pt_zone);
		previous->next = pt_to_save;
		//on modifie le next du précedent pour le faire pointer sur la valeur du next sauvegardé en mémoire
		ob* pt_ob;
		pt_ob = (ob*) pt_zone;
		pt_ob->size = pt_zone->size;
		return(pt_ob);
		//on alloue le fb --> transformation en ob


		
	}
	
	//--> on alloue et il reste de la place derrière pour créer une zone libre
	//cas plus complexe
	else if(pt_zone->size - taille > sizeof(fb)){
			//on récupère l'adresse après la zone qui deviendra occupé 
			fb* new_free = pt_zone+taille;
			// on récupère l'adresse de la zone précédente
			fb* previous = getPrevious(pt_zone);
			//on fait pointer le précédent next sur le nouveau free
			previous->next = new_free;
			//la nouvelle zone de libre (plus petite) pointe sur le next de la zone libre précédente
			new_free->next = pt_zone->next;
			//modification de la taille de la zone libre
			new_free->size = pt_zone->size-taille;
			//on creer le pointeur sur la zone occupé
			ob* pt_ob;
			pt_ob = (ob*) pt_zone;
			//la taille de la zone occupé est celle que l'on veut affecter
			pt_ob->size = taille;
			//on return le pointeur
			return(pt_ob);
	}
	//si aucune zone mémoire n'a pu etre initialisé

	//fb* this_fb=get_header()->fit(/*...*/NULL, /*...*/0);
	/* ... */
	return NULL;
}

fb* mem_fit_first(fb *list, size_t size) {
	//la fonction doit renvoyer l'adresse du premier bloc libre >= size  dans les blocs libre present dans l'adresse list
	fb *pt_mem = list;
	//return NULL si le bloc n'existe pas
    // sinon renvoyer l'adresse de la struct present dans la liste
    //si la liste est null, on return null
    if(list == NULL){
	   return NULL;
    }
	while ((void*) pt_mem < memory_addr+get_header()->memory_size) {
		//boucle pour parcourir tout le gros bloc de départ
		if(pt_mem->size >= size){
			//si la taille que pointe le pointeur est suffisament grand on renvoie
			return pt_mem;
		}else{
			//sinon on pointe le pointeur suivant
			pt_mem = pt_mem->next;
		}


	}



	return NULL;
}

fb* getPrevious(fb* a_pour_previous){
	//on renvoie l'adresse du fb précédent:
		fb *pt_mem = get_header()->first_fb;
		//on recupère dans un pointeur la prochaine struc fb
		//si on est dans le cas ou on on demande le previous du premier fb
		//on return null
		if(a_pour_previous == pt_mem){ //pt_mem =  get_header()->first_fb;
			return NULL;
		}

		while ((void*) pt_mem < memory_addr+get_header()->memory_size) {
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
