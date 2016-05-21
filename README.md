
# Libstl - A very small and fast library for STL files

[![Build Status](https://travis-ci.org/aki5/libstl.svg?branch=master)](https://travis-ci.org/aki5/libstl)

Libstl provides basic functions for reading and writing STL files, but instead of providing just the
parsing function, libstl includes easy to use functions to build more meaningful data structures from the
STL data.

Repairing STL files is not one of the goals of libstl, because that would require changing the input
geometry and the topology it implies. Instead, from libstl point of view, an application for repairing
STL files would probably benefit from using libstl.

## The half-edge data structure in libstl

Libstl does not declare a `struct` to represent edges. Instead, a half-edge is an immutable unsigned integer.
To declare and access half-edge attributes, the half-edges are used as array indices. This makes the half-edge
open-ended, applications can add their own attributes by just declaring their own arrays and indexing them
with the half-edges.

The half-edges are created in groups of two, so that accessing the opposing half-edge is
a simple `edge^1` expression, flipping the least significant bit.

![Half-Edge data structure](https://raw.githubusercontent.com/aki5/libstl/master/half-edges.png)

The figure above illustrates this principle. The `next` array contains loops around the faces
of the mesh. At every index, the `next` array contains the following half-edge. Likewise, the
`vert` array contains a vertex identifier corresponding to the source of each half-edge.

## Loadstl function

_Loadstl_ reads in an STL (stereolithography) file and returns the components of an indexed triangle mesh, together
with attributes and normal vectors from the stl file. _Loadstl_ will allocate memory for all of the arrays it
returns.

Duplicate vertices in the file are merged based on exact equivalence, there is no welding threshold. If the input
file contains holes or cracks, they will be present in the indexed triangle mesh that _loadstl_ returns.

```
int loadstl(FILE *fp, float **vertp, uint32_t *nvertp, uint32_t **trip, uint16_t **attrp, uint32_t *ntrip);
```

The arguments to _loadstl_ are:
* FILE *fp: the stdio FILE to read the stl from. Fp will not be closed, and if there is anything in the file after the
stl data, it can be read from fp after _loadstl_ returns. Fp does not need to be seekable.
* float **vertp: location for storing the the vertex coordinates
* uint32_t *nvertp: location for storing the number of vertices
* uint32_t **trip: location for storing the triangles
* uint16_t **attrp: location for storing the triangle attributes
* uint32_t *ntrip: location for storing the number of triangles read

## Halfedges function

_Halfedges_ computes standard half-edges from a triangle array, where the values are vertex identifiers.
The half-edges will be joined based on vertex identifiers of two triangles coinciding, so vertices need
to be shared by connected triangles, or the output will contain orphan half-edges.

An orphan half-edge has the value `~(uint32_t)0`, also defined as the macro `BADHALFEDGE`.

```
int halfedges(uint32_t *tris, uint32_t ntris, uint32_t **nextp, uint32_t **vertp, uint32_t *nedgep);
```

The returned arrays are as follows
* uint32_t **nextp, index of the next half-edge around a face, indexed by the half-edge id.
* uint32_t **vertp, index of the source vertex for a half-edge, indexed by the half-edge id
* uint32_t *nedgep, the number of half-edges

## Dualedges function

```
int dualedges(uint32_t *enext, uint32_t nedges, uint32_t **vnextp);
```

_Dualedges_ computes the dual topology of the input half-edge array, meaning that if the input edges
are chained around the faces, dualedges will return the same number of half-edges, but the chains go
around vertices. Together, halfedges and dualedges arrays form the quad-edge data structure.

