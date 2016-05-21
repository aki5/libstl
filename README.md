
# libstl, A very small and fast library for dealing with stl files

[![Build Status](https://travis-ci.org/aki5/libstl.svg?branch=master)](https://travis-ci.org/aki5/libstl)

```
// load stl file, compute and return an indexed triangle mesh
int loadstl(FILE *fp, float **vertp, uint32_t *nvertp, uint32_t **trip, uint16_t **attrp, uint32_t *ntrip);
```

_Loadstl_ reads in an STL (stereolithography) file and returns the components of an indexed triangle mesh, together
with attributes and normal vectors from the stl file. _Loadstl_ will allocate memory for all of the arrays it
returns.

Duplicate vertices in the file are merged based on exact equivalence, there is no welding threshold. If the input
file contains holes or cracks, they will be present in the indexed triangle mesh that _loadstl_ returns.

The arguments to _loadstl_ are:
* FILE *fp: the stdio FILE to read the stl from. Fp will not be closed, and if there is anything in the file after the
stl data, it can be read from fp after _loadstl_ returns. Fp does not need to be seekable.
* float **vertp: location for storing the the vertex coordinates
* uint32_t *nvertp: location for storing the number of vertices
* uint32_t **trip: location for storing the triangles
* uint16_t **attrp: location for storing the triangle attributes
* uint32_t *ntrip: location for storing the number of triangles read

```
// compute halfedges from an indexed triangle mesh
int halfedges(uint32_t *tris, uint32_t ntris, uint32_t **nextp, uint32_t **vertp, uint32_t *nedgep);
```

_Halfedges_ computes standard half-edges from an indexed triangle mesh. The returned arrays are as follows
* uint32_t **nextp, index of the next half-edge around a face, indexed by the half-edge id.
* uint32_t **vertp, index of the source vertex for a half-edge, indexed by the half-edge id
* uint32_t *nedgep, number of edges returned, half the number of half-edges

```
// compute dual edges from half edges (return vertex loops if passed face loops, vice versa)
int dualedges(uint32_t *enext, uint32_t nedges, uint32_t **vnextp);
```

_Dualedges_ computes the dual topology of the input half-edge array, meaning that if the input edges
are chained around the faces, dualedges will return the same number of half-edges, but the chains go
around vertices. This can be useful for subdivisions.
