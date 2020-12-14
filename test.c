#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void afficher_zone(void *adresse, size_t taille, int free)
{
  printf("Zone %s, Adresse : %lu, Taille : %lu\n", free?"libre":"occupee",
         adresse - get_memory_adr(), (unsigned long) taille);
}

void test1() {
  printf("=======================================\n");

  mem_init(get_memory_adr(), get_memory_size());

  printf("On alloue la moitié de la memoire\n");
  void* ptr1 = mem_alloc(get_memory_size()/2);
  mem_show(afficher_zone);

  printf("\nEt on la libere\n");
  mem_free(ptr1);
  mem_show(afficher_zone);

}

void test2() {
  printf("=======================================\n");

  mem_init(get_memory_adr(), get_memory_size());

  printf("On alloue deux zones\n");
  void* ptr1 = mem_alloc(64);
  void* ptr2 = mem_alloc(64);
  mem_show(afficher_zone);

  printf("\nOn libère la premiere zone\n");
  mem_free(ptr1);
  mem_show(afficher_zone);

  printf("\nOn libere la deuxième zone en fusionant les deux autres\n");
  mem_free(ptr2);
  mem_show(afficher_zone);
}

void test3() {
  printf("=======================================\n");

  mem_init(get_memory_adr(), get_memory_size());

  printf("On alloue trois zones\n");
  /*void* ptr1 = */mem_alloc(64);
  void* ptr2 = mem_alloc(64);
  /*void* ptr3 = */mem_alloc(64);
  mem_show(afficher_zone);

  printf("\nOn suprime celle du milieu, creant ainsi une nouvelle zone\n");
  mem_free(ptr2);
  mem_show(afficher_zone);
}

void test4() {
  printf("=======================================\n");

  mem_init(get_memory_adr(), get_memory_size());

  printf("On une premiere zone\n");
  void* ptr1 = mem_alloc(64);
  mem_show(afficher_zone);

  printf("\nOn alloue tout le reste dans une deuxieme zone\n");
  void* ptr2 = alloc_max(get_memory_size());
  mem_show(afficher_zone);

  printf("\nOn libere la premiere zone\n");
  mem_free(ptr1);
  mem_show(afficher_zone);

  printf("\nOn libere le reste\n");
  mem_free(ptr2);
  mem_show(afficher_zone);
}

int main(int argc, char const *argv[]) {
  test1();
  test2();
  test3();
  test4();
  return 0;
}
