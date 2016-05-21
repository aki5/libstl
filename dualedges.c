#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stlfile.h"

// when given an edge array where edges loop around faces, returns an
// array where edges loop around vertices
int
dualedges(uint32_t *enext, uint32_t nedges, uint32_t **vnextp)
{
	uint32_t ei;
	uint32_t nverts;
	uint32_t *vnext;

	vnext = malloc(nedges * 2*sizeof vnext[0]);
	for(ei = 0; ei < 2*nedges; ei++)
		vnext[ei] = ~(uint32_t)0;

	nverts = 0;
	for(ei = 0; ei < 2*nedges; ei++){
		uint32_t edge;
		edge = ei;
		if(vnext[edge] == ~(uint32_t)0){
			do {
				uint32_t tmp;
				tmp = enext[edge^1]; // next on the opposite face.
				vnext[edge] = tmp;
				edge = tmp;
			} while(edge != ei);
			nverts++;
		}
	}

	fprintf(stderr, "dualedges: found %d loops\n", nverts);

	*vnextp = vnext;
	return nverts;
}
