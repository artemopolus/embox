// NanoRT defines template based class, so no NANORT_IMPLEMENTATION anymore.
#include "nanort.h"







int main(int argc, char *argv[]) {


  Mesh mesh;
  // load mesh data...
  nanort::BVHBuildOptions<float> options; // Use default option
  nanort::TriangleMesh<float> triangle_mesh(mesh.vertices, mesh.faces, /* stride */sizeof(float) * 3);
  nanort::TriangleSAHPred<float> triangle_pred(mesh.vertices, mesh.faces, /* stride */sizeof(float) * 3);
  nanort::BVHAccel<float> accel;
  ret = accel.Build(mesh.vertices, mesh.faces, mesh.num_faces, options);
  assert(ret);
  nanort::BVHBuildStatistics stats = accel.GetStatistics();
  printf("  BVH statistics:\n");
  printf("    # of leaf   nodes: %d\n", stats.num_leaf_nodes);
  printf("    # of branch nodes: %d\n", stats.num_branch_nodes);
  printf("  Max tree depth   : %d\n", stats.max_tree_depth);

  std::vector<float> rgb(width * height * 3, 0.0f);
  const float tFar = 1.0e+30f;
  // Shoot rays.
  #ifdef _OPENMP
  #pragma omp parallel for
  #endif
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      BVHTraceOptions trace_options;
      // Simple camera. change eye pos and direction fit to .obj model.
      nanort::Ray<float> ray;
      ray.min_t = 0.0f;
      ray.max_t = tFar;
      ray.org[0] = 0.0f;
      ray.org[1] = 5.0f;
      ray.org[2] = 20.0f;
      float3 dir;
      dir[0] = (x / (float)width) - 0.5f;
      dir[1] = (y / (float)height) - 0.5f;
      dir[2] = -1.0f;
      dir.normalize();
      ray.dir[0] = dir[0];
      ray.dir[1] = dir[1];
      ray.dir[2] = dir[2];

      nanort::TriangleIntersector<> triangle_intersecter(mesh.vertices, mesh.faces, /* stride */sizeof(float) * 3);
      nanort::TriangleIntersection<> isect,
      bool hit = accel.Traverse(ray, triangle_intersector, &isect, trace_options);
      if (hit) {
        // Write your shader here.
        float3 normal;
        unsigned int fid = triangle_intersector.intersect.prim_id;
        normal[0] = mesh.facevarying_normals[3*3*fid+0]; // @todo { interpolate normal }
        normal[1] = mesh.facevarying_normals[3*3*fid+1];
        normal[2] = mesh.facevarying_normals[3*3*fid+2];
        // Flip Y
        rgb[3 * ((height - y - 1) * width + x) + 0] = fabsf(normal[0]);
        rgb[3 * ((height - y - 1) * width + x) + 1] = fabsf(normal[1]);
        rgb[3 * ((height - y - 1) * width + x) + 2] = fabsf(normal[2]);
      }
    }
  }



  return 0;
}
