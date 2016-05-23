
# A decent library for STL files

[![Build Status](https://travis-ci.org/aki5/libstl.svg?branch=master)](https://travis-ci.org/aki5/libstl)
[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/aki5/libstl/master/LICENSE)

Libstl provides basic functions for reading and writing STL files, but instead of providing just the parsing function, libstl includes easy to use functions to build more meaningful data structures from the raw triangle data.

## Libstl is still work in progress, but

* It is small and fast, suitable for embedded systems,
* increasingly well documented (keep reading),
* un-opinionated and easy to extend,
* returns an indexed triangle mesh,
* can compute half-edge or quad-edge data structures,
* supports 15-bit triangle colors,
* treats concatenation of multiple STL files as a polyhedral complex
	* keep calling `loadstl()` until it fails, 
	* comment field defines per polyhedron attributes,
	* simplest data format for defining multi-material 3d prints

Repairing STL files is not one of the goals of _libstl_, because that would require subjectively changing the input geometry and the topology it implies. Instead, from _libstl_ point of view, an application for repairing STL files would probably benefit from using it.

Below is an example program for loading a file and computing half-edges and dual-edges for it.

```
#include <stdio.h>
#include <stdint.h>
#include "stlfile.h"

int main(void){
	char comment[80];
	FILE *fp;
	float *vertpos;
	triangle_t *tris, ntris;
	vertex_t nverts;
	uint16_t *triattrs;

	fp = fopen("path/to/file.stl", "rb");
	loadstl(fp, comment, &vertpos, &nverts, &tris, &triattrs, &ntris);
	fclose(fp);

	halfedge_t *fnext, *vnext, nedges;
	vertex_t *evert;
	
	halfedges(tris, ntris, &fnext, &evert, &nedges);
	dualedges(fnext, nedges, &vnext);

	return 0;
}
```

## Data types

There are three basic data types: a _vertex_, a _triangle_ and a _half-edge_. All of them are unsigned integers, and each represents a _unique identifier_ for an object of the said type. That is, we define abstract identifiers for the types that are separate from algorithm specific details of what it _means_ to be a _vertex_, a _triangle_ or a _half-edge_.

By convention, _unique identifiers_ start from 0 and run sequentially. ~0 is used as an _invalid identifier_ where needed. Each type has its own identifier space starting from 0. Functions don't typically return arrays of these identifiers, but instead return a value telling how many triangles were used.

Vertex positions are stored as an array of floats, and a coordinate of a vertex can be accessed by indexing the position array with the _vertex_ itself. As a technical detail, the identifier may sometimes need to be scaled when accessing an array, like in the example below, where the coordinate array has 3 floats per vertex (x, y and z).

```
x = vertpos[3*vert];
y = vertpos[3*vert+1];
z = vertpos[3*vert+2];
```

If the `vertpos` array consisted of `structs` with `x`, `y` and `z` fields, the multiplication would not be necessary. However, using flat arrays like above makes it easier to tell _OpenGL_ where and how to fetch vertex data.

By focusing on stable identifiers instead of a specific set of attributes  makes the library more open-ended: algorithms can easily add attributes to any object just by declaring temporary arrays and indexing them during computation.

It would be nice to get type errors for using a _vertex_ as an offset to an attribute array for _half-edges_ for example, but the type system of standard C is too weak to express that.

## The indexed triangle mesh structure

The `loadstl` function in _libstl_ return an indexed triangle meshes, where the vertex data is in one array with duplicate vertex coordinates merged, and a _triverts_ array a group of three _vertices_ express the corners for each individual triangle.

As discussed in the previous section, only the number of vertices and triangles is returned, numbers counting from 0 up to that number are valid triangles.

```
int loadstl(FILE *fp, char *comment, float **vertp, vertex_t *nvertp, vertex_t **trivertp, uint16_t **attrp, triangle_t *ntrip);
```

![Triangle mesh structure](https://raw.githubusercontent.com/aki5/libstl/master/triangle-mesh.png)

Due to not having declared a `struct` for the triangle corners, the _triangle_ needs to be scaled by 3 when accessing the `triverts` array, as follows

```
v0 = triverts[3*tri];
v1 = triverts[3*tri+1];
v2 = triverts[3*tri+2];
```

This is again a relatively common way of representing indexed triangle meshes to _OpenGL_.

Often, applications find that they need to create duplicates of some vertices, because while they share the same position, they differ in some other attribute. _Libstl_ is mostly concerned with topology defined by position sharing, and leaves the details of vertex duplication to applications.

## The half-edge data structure

A half-edge data structure consists of directed edges, conventionally looping counterclockwise around faces when looking from the outside in. A half-edge typically has at least the following properties

* a _source vertex_,
* a _next half-edge_ (around left face when looking toward destination from the source), and
* an _adjacent half-edge_ (opposite direction, loops around _adjacent face_).

In a well-formed triangle mesh (orientable 2-manifold), there are no orphan half-edges, which simply tells us that there are no cracks or holes in the surface and that every vertex (and edge) takes part in only one polyhedron.

_Libstl_ takes the view that a single STL file should define a single solid object, with polyhedral complexes represented as a concatenation of multiple STL files. As such, using half-edges to represent connectivity in an individual STL files should not be an issue even for multi-material applications.

In _libstl_, _half-edges_ are created in pairs so that accessing the _adjacent half-edge_ is achieved by flipping the least significant bit of the _half-edge_, ie. `edge^1`.

```
int halfedges(vertex_t *triverts, triangle_t ntris, halfedge_t **nextp, vertex_t **vertp, halfedge_t *nedgep);
```

![Half-Edge data structure](https://raw.githubusercontent.com/aki5/libstl/master/half-edges.png)

The figure above illustrates this principle. The `next` array contains loops around the faces of the mesh. At every index, the `next` array contains the following half-edge. Likewise, the `vert` array contains a vertex identifier corresponding to the source of each half-edge.

As an example, `vert[edge]` returns the source vertex of an edge, while `vert[edge^1]` returns the source vertex of the opposing edge (aka. the destination of current edge). As a more complicated example, looping around edges of a face can be written as follows.

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

