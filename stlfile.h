
// load stl file, compute and return an indexed triangle mesh
int loadstl(FILE *fp, float **vertp, uint32_t *nvertp, uint32_t **trip, uint16_t **attrp, uint32_t *ntrip);

// compute halfedges from an indexed triangle mesh
int halfedges(uint32_t *tris, uint32_t ntris, uint32_t **nextp, uint32_t **vertp, uint32_t *nedgep);

// compute dual edges from half edges (return vertex loops if passed face loops, vice versa)
int dualedges(uint32_t *enext, uint32_t nedges, uint32_t **vnextp);
