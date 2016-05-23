
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include "stlfile.h"

int
main(int argc, char *argv[])
{
	FILE *fp;
	char comment[80];
	float *verts;
	vertex_t *tris;
	halfedge_t *enext;
	vertex_t *evert;
	halfedge_t *vnext;
	uint16_t *attrs;
	triangle_t ntris;
	halfedge_t nedges;
	vertex_t nverts;
	int i, nrounds, opt;

	nrounds = 5;
	while((opt = getopt(argc, argv, "n:")) != -1){
		switch(opt){
		case 'n':
			nrounds = strtol(optarg, NULL, 10);
			break;
		default:
		caseusage:
			fprintf(stderr, "usage: %s [-n nrounds] path/to/file.stl\n", argv[0]);
			exit(1);
		}
	}
	if(optind == argc)
		goto caseusage;

	for(i = 0; i < nrounds; i++){
		if(argc > 1){
			fp = fopen(argv[optind], "rb");
		} else {
			fp = stdin;
		}
		loadstl(fp, comment, &verts, &nverts, &tris, &attrs, &ntris);
		halfedges(tris, ntris, &enext, &evert, &nedges);
		dualedges(enext, nedges, &vnext);
		fclose(fp);
		free(tris);
		free(verts);
		free(attrs);
		free(enext);
		free(evert);
		free(vnext);
	}

	return 0;
}
