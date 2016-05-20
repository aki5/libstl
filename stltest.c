
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stlfile.h"

int
main(int argc, char *argv[])
{
	FILE *fp;
	float *verts;
	uint32_t *tris;
	uint32_t *enext, *evert;
	uint16_t *attrs;
	uint32_t ntris, nedges, nverts;
	int i;

	for(i = 0; i < 5; i++){
		if(argc > 1){
			fp = fopen(argv[1], "rb");
		} else {
			fp = stdin;
		}
		loadstl(fp, &verts, &nverts, &tris, &attrs, &ntris);
		halfedges(tris, ntris, &enext, &evert, &nedges);
		fclose(fp);
		free(tris);
		free(verts);
		free(attrs);
		free(enext);
		free(evert);
	}

	return 0;
}
