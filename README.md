
# Libstl - A very small and fast library for STL files

[![Build Status](https://travis-ci.org/aki5/libstl.svg?branch=master)](https://travis-ci.org/aki5/libstl)

Libstl provides basic functions for reading and writing STL files, but instead of providing just the parsing function, libstl includes easy to use functions to build more meaningful data structures from the raw triangle data.

Repairing STL files is not one of the goals of libstl, because that would require subjectively changing the input geometry and the topology it implies. Instead, from libstl point of view, an application for repairing STL files would probably benefit from using libstl.

Below is an example program for loading an stl file and computing half-edges and dual-edges for it.

```
#include <stdio.h>
#include "stlfile.h"
int main(void){
	float *vertpos;
	triangle_t *tris, ntris;
	halfedge_t *fnext, *vnext, nedges;
	vertex_t *evert, nverts;
	
	fp = fopen("path/to/file.stl", "rb");
	loadstl(fp, &vertpos, &nverts, &tris, NULL, &ntris);
	fclose(fp);
	halfedges(tris, ntris, &fnext, &evert, &nedges);
	dualedges(fnext, nedges, &vnext);
	return 0;
}
```

## Data types

There are three basic data types: a _vertex_, a _triangle_ and a _half-edge_. All of them are unsigned integers, and each represents a _unique identifier_ for an object of the said type. That is, we define abstract identifiers for the types that are separate from algorithm specific details of what it means to be a _vertex_, a _triangle_ or a _half-edge_.

By convention, _unique identifiers_ start from 0 and run sequentially. ~0 is used as an _invalid identifier_ where needed. Each type has its own identifier space starting from 0.

Vertex positions are stored as an array of floats, and a coordinate of a vertex can be accessed by indexing the position array with the _vertex_ itself. As a technical detail, the identifier may sometimes need to be scaled when accessing an array, like in the example below, where the coordinate array has 3 floats per vertex (x, y and z).

```
x = vertpos[3*vert];
y = vertpos[3*vert+1];
z = vertpos[3*vert+2];
```

If the `vertpos` array contained structs with `x`, `y` and `z` fields instead, the multiplication would not be necessary. However, using flat arrays like above makes it easier to tell _OpenGL_ how to fetch vertex data.

By restricting ourselves to identifiers makes the library more open-ended, algorithms can easily augment vertices or half-edges with their own data by declaring temporary arrays and indexing them during computation.

It would be nice to get type errors for using a _vertex_ as an offset to an attribute array for _half-edges_ for example, but the type system of standard C is too weak to express that.

## The indexed triangle mesh structure

The file loaders and savers in _libstl_ return indexed triangle meshes, where the vertex data is in one array with duplicate vertex coordinates merged, and a _triverts_ array where groups of three _vertices_ express the corners of individual triangles.

![Triangle mesh structure](https://raw.githubusercontent.com/aki5/libstl/master/triangle-mesh.png)

This is a relatively common way of representing indexed triangle meshes in an _OpenGL_ applications. More often than not, applications will need to create duplicates of vertices that share the same position, because the vertex differs in some other attribute in one or more triangles sharing the same vertex position.

_Libstl_ is mostly concerned with mesh topology with respect to shared vertex positions, leaving the details of duplicating vertices for rendering purposes to the applications.

## The half-edge data structure

Libstl does not declare a struct to represent edges. Instead, a half-edge is an immutable unsigned integer. To declare and access half-edge attributes, the half-edges are used as array indices. 

The half-edges are created in groups of two, so that accessing the opposing half-edge is a simple `edge^1` expression, flipping the least significant bit.

![Half-Edge data structure](https://raw.githubusercontent.com/aki5/libstl/master/half-edges.png)

The figure above illustrates this principle. The `next` array contains loops around the faces of the mesh. At every index, the `next` array contains the following half-edge. Likewise, the `vert` array contains a vertex identifier corresponding to the source of each half-edge.

As an example, accessing the source vertex of an edge would be `vert[edge]`, and the destination would be accessed as `vert[edge^1]`. Looping across all edges around a face could be written as follows.

```
uint start;
start = edge;
do {
        edge = next[edge];
} while(edge != start);
```

## The quad-edge data structure

The conventional quad-edge consists of four `next` pointers, where each pointer participates in a list of edges around a geometric entity.

Two of the pointers in a quad-edge correspond to the `next` array in half-edges. The remaining two correspond to `next` arrays on the dual graph, where outgoing edges from a vertex are chained together in a counter-clockwise direction.

The navigation operations of the traditional quad-edge data structure are the same as those in the half-edge data structure, with the addition of a `rotate` operation, which moves between the face-loop and edge-loop graphs.

We provide a `dualedges` function, which computes the dual graph. Quad-edges can be quite naturally represented as two separate `next` arrays indexed by the half-edges, say `fnext` and `vnext`, corresponding to _next around the right face_ and _next around source vertex_ respectively. In this model, the dual graph can be passed to a function call by simply calling it with the array arguments swapped.

Merging the two arrays to form a standard quad-edge is not very difficult either, giving a structure where a full edge is a set of four consecutive integers and slightly different arithmetic from the half-edges.

## Loadstl function

_Loadstl_ reads in an STL (stereolithography) file and returns the components of an indexed triangle mesh, together with attributes and normal vectors from the stl file. _Loadstl_ will allocate memory for all of the arrays it returns.

Duplicate vertices in the file are merged based on exact equivalence, there is no welding threshold. If the input file contains holes or cracks, they will be present in the indexed triangle mesh that _loadstl_ returns.

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

An orphan half-edge has the value `~(uint32_t)0`.

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

