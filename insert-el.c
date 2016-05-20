/* Copyright Sriganesh Kini. May 2016. All rights reserved */
/* Reference: draft-ietf-mpls-spring-entropy-label */
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#define MAX_LSE 30

int main (int argc, char **argv) {
  struct lse { int rld; int elc; } * lse = calloc(sizeof(struct lse), MAX_LSE);
  int max_el_insert_capability = INT_MAX;
  int num_el_inserted = 0;
  int num_lse = 0;
  int cur_el_insertion_point;
  int i, new_el_insertion_point, max_new_el_insertion_point;
  int closest_elc;

  /* The RLD of a LSE should allow load-balancing at the maximum number of transit
     LSRs along the path where that LSE is looked-up to switch packets. So if the LSE
     is used to switch packets along shortest paths, then it should be the min of
     the RLD of LSRs along those paths */
  if (argc < 2) {
    printf("Usage: %s <max EL insert capability, use -1 for no limit> "
	   "<label stack entry1 ELC> <label stack entry1 RLD, use -1 for no limit> "
	   "<label stack entry2 ELC> <label stack entry2 RLD> .../* list LSEs bottom to top*/\n", argv[0]);
    exit(-1);
  }

  sscanf(argv[1], "%d", &max_el_insert_capability);
  if (max_el_insert_capability < 0)
    max_el_insert_capability = INT_MAX;
  
  for (i=2; i < argc; i++) {
    if (i%2) {
      sscanf(argv[i], "%d", &lse[num_lse].rld);
      if (lse[num_lse].rld < 0) lse[num_lse].rld = INT_MAX;
      num_lse++;
    } else
      sscanf(argv[i], "%d", &lse[num_lse].elc);
  }

  /* Initialize the current EL insertion point to the
     bottommost label in the stack that is EL-capable */
  for (cur_el_insertion_point=0; cur_el_insertion_point < num_lse; cur_el_insertion_point++)
    if (lse[cur_el_insertion_point].elc) break;

  while (num_el_inserted < max_el_insert_capability && cur_el_insertion_point < num_lse ) {
    num_el_inserted++; printf("Insert ELI, EL #%d below %d ", num_el_inserted, cur_el_insertion_point);

    /* Locate the lse that cannot use the currently inserted EL */
    new_el_insertion_point = cur_el_insertion_point;
    for (i = cur_el_insertion_point+1; i < num_lse; i++) {

      /* The shallowest ELC position to insert new EL */
      if (lse[i].elc) 
	new_el_insertion_point = i;

      /* If RLD is not large enough to read current EL AND
	 there is an ELC lse shallower than current EL that is within
	 the RLD of this LSE then make it the new position to add EL */
      if (lse[i].rld < (i+1) - cur_el_insertion_point + 2 &&
	  new_el_insertion_point > cur_el_insertion_point &&
	  lse[i].rld >= (i+1) - new_el_insertion_point + 2) {
	  cur_el_insertion_point = new_el_insertion_point;	
	break;
      }
    }
    if (i >= num_lse)
      break;
  }
  printf("\n");
  assert(num_el_inserted <= max_el_insert_capability);
}
