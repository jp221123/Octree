# Octree

Dynamic octree implementation with openGL.

## Supported objects
- Spheres of various sizes
- Axis-aligned cubes of various sizes

## Supported queries of octrees
- Insertion, update, or removal of an object
- Collision test for an object (against the objects in the octree)
- Collision test for a ray (or rather an oriented segment)

## Supported functions
- Toggle whether all objects randomly move (to show the efficiency) or not
- Select the objects and manually translate them: an object stops moving once it collides with something else
- Show/hide the octree structure
- Move the camera

## Demo and usage
The usage can be found in the binary file.

### Demo - big
10,000 objects

Octree node capacity = 10 (default)

### Demo - small
100 objects

Octree node capacity = 1 (to show the structure changes better)

## Some design decisions
There are some design decisions to make when implementing octrees.
- The default capacity of an octree node is 10 (though rather arbitrarily determined). That is, when a leaf node intersects with more than 10 objects, it is subdivided into 8 nodes and becomes an internal node.
- Each leaf node maintains a list of objects intersecting to its bounding box.
- The octree doesn't allow intersecting objects to be inserted at all.
- Thus, persistency is delegated to objects.

## Possible improvements

- Implement compressed octrees or even skip compressed octrees.

Note that all objects are fat right now. The bound for octrees depends on the spread factor of objects, i.e., when there can be arbitrarily skinny objects, octrees will not work well.

- Generate the octree subdivision lines only with the debug argument.

The slow-down caused by the number of subdivision lines is not severe but still noticeable, but the current code needs to be refactored not to draw those lines.

- Support the range frustum queries for left click drag.

- Support the rotation of objects.

- Support the arbitrarily oriented cubes.


## References

Basic openGL start-up codes are brought from: [Link][opengl]

[opengl]: http://www.opengl-tutorial.org/

Icosphere-mesh-generating code is adapted from: [Link][songho]

[songho]: http://www.songho.ca/opengl/gl_sphere.html#icosphere

Blinn-phong shaders are brought from: [Link1][phong] and [Link2][blinn]

[phong]: http://www.cs.toronto.edu/~jacobson/phong-demo/

[blinn]: https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model#OpenGL_Shading_Language_code_sample
