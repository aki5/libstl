#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "hash64.h"

static uint32_t
edge(uint32_t *edges, uint32_t nedges, uint32_t *eht, uint32_t ehtcap, uint32_t *e)
{
	uint32_t *eip, ei;
	uint32_t hash;
	uint32_t i;

	hash = final64(e[0], e[1]);
	for(i = 0; i < ehtcap; i++){
		eip = eht + ((hash + i) & (ehtcap - 1));
		ei = *eip;
		if(ei == 0){
			*eip = nedges+1;
			return nedges;
		}
		ei--;
		if(cmp64(e, edges + 2*ei) == 0)
			return ei;
	}
	return ~(uint32_t)0;
}

int
halfedges(uint32_t *tris, uint32_t ntris, uint32_t **nextp, uint32_t **vertp, uint32_t *nedgep)
{
	uint32_t *next, *vert;
	uint32_t *edges, *eht;
	uint32_t nedges, ehtcap;
	uint32_t i, ti;

	nedges = 0;
	next = malloc(3*ntris * 2*sizeof next[0]);
	vert = malloc(3*ntris * 2*sizeof vert[0]);

	edges = malloc(3*ntris * 2*sizeof edges[0]);
	ehtcap = nextpow2(3*ntris);
	eht = malloc(ehtcap * sizeof edges[0]);
	memset(eht, 0, ehtcap * sizeof edges[0]);
	for(i = 0; i < ntris; i++){
		uint32_t vloop[3];
		uint32_t eloop[3];
		for(ti = 0; ti < 3; ti++){
			uint32_t v0, v1, e[2];
			uint32_t ei;
			v0 = tris[3*i+ti];
			v1 = tris[3*i+((1<<ti)&3)];
			e[0] = v0 < v1 ? v0 : v1; // min
			e[1] = v0 < v1 ? v1 : v0; // the other
			ei = edge(edges, nedges, eht, ehtcap, e);
			if(ei == ~(uint32_t)0){
				fprintf(stderr, "halfedge: hash full at triangle %d/%d cap %d\n", i, ntris, ehtcap);
				goto exit_fail;
			}
			if(ei == nedges){
				// number of edges increased,
				// 'lazy' initialize entries to known bad indices.
				next[2*ei] = ~(uint32_t)0;
				next[2*ei+1] = ~(uint32_t)0;
				vert[2*ei] = ~(uint32_t)0;
				vert[2*ei+1] = ~(uint32_t)0;
				copy64(edges + 2*nedges, e);
				nedges++;
				eloop[ti] = 2*ei;
			} else {
				eloop[ti] = 2*ei+1;
			}
			vloop[ti] = v0;
		}
		for(ti = 0; ti < 3; ti++){
			next[eloop[ti]] = eloop[(1<<ti)&3];
			vert[eloop[ti]] = vloop[ti];
		}
	}

	// scan through, warn about orphan edges.
	for(i = 0; i < 2*nedges; i++){
		if(next[i] == ~(uint32_t)0){
			fprintf(stderr, "orphan edge");
			next[i] = i;
		}
	}

	free(eht);
	free(edges);
	next = realloc(next, nedges * 2*sizeof next[0]);
	vert = realloc(vert, nedges * 2*sizeof vert[0]);
	*nextp = next;
	*vertp = vert;
	*nedgep = nedges;
	fprintf(stderr, "halfedge: found %d edges\n", nedges);
	return 0;

exit_fail:
	free(eht);
	free(edges);
	free(next);
	free(vert);
	return -1;
}
