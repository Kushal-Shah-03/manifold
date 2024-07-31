// Copyright 2021 The Manifold Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>

#include "cross_section.h"
#include "manifold.h"
#include "samples.h"
#include "sdf.h"
#include "test.h"
#include "tri_dist.h"

using namespace manifold;

TEST(Hull, Tictac) {
  const float tictacRad = 100;
  const float tictacHeight = 500;
  const int tictacSeg = 1000;
  const float tictacMid = tictacHeight - 2 * tictacRad;
  const auto sphere = Manifold::Sphere(tictacRad, tictacSeg);
  const std::vector<Manifold> spheres{sphere,
                                      sphere.Translate({0, 0, tictacMid})};
  const auto tictac = Manifold::Hull(spheres);

#ifdef MANIFOLD_EXPORT
  if (options.exportModels) {
    ExportMesh("tictac_hull.glb", tictac.GetMesh(), {});
  }
#endif

  EXPECT_EQ(sphere.NumVert() + tictacSeg, tictac.NumVert());
}

#ifdef MANIFOLD_EXPORT
TEST(Hull, Fail) {
  Manifold body = ReadMesh("hull-body.glb");
  Manifold mask = ReadMesh("hull-mask.glb");
  Manifold ret = body - mask;
  MeshGL mesh = ret.GetMesh();
}
#endif

TEST(Hull, Hollow) {
  auto sphere = Manifold::Sphere(100, 360);
  auto hollow = sphere - sphere.Scale({0.8, 0.8, 0.8});
  const float sphere_vol = sphere.GetProperties().volume;
  EXPECT_FLOAT_EQ(hollow.Hull().GetProperties().volume, sphere_vol);
}

TEST(Hull, Cube) {
  std::vector<glm::vec3> cubePts = {
      {0, 0, 0},       {1, 0, 0},   {0, 1, 0},      {0, 0, 1},  // corners
      {1, 1, 0},       {0, 1, 1},   {1, 0, 1},      {1, 1, 1},  // corners
      {0.5, 0.5, 0.5}, {0.5, 0, 0}, {0.5, 0.7, 0.2}  // internal points
  };
  auto cube = Manifold::Hull(cubePts);
  EXPECT_FLOAT_EQ(cube.GetProperties().volume, 1);
}

TEST(Hull, Empty) {
  const std::vector<glm::vec3> tooFew{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
  EXPECT_TRUE(Manifold::Hull(tooFew).IsEmpty());

  const std::vector<glm::vec3> coplanar{
      {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}};
  EXPECT_TRUE(Manifold::Hull(coplanar).IsEmpty());
}

TEST(Hull, MengerSponge) {
  Manifold sponge = MengerSponge(4);
  sponge = sponge.Rotate(10, 20, 30);
  Manifold spongeHull = sponge.Hull();
  EXPECT_EQ(spongeHull.NumTri(), 12);
  EXPECT_FLOAT_EQ(spongeHull.GetProperties().surfaceArea, 6);
  EXPECT_FLOAT_EQ(spongeHull.GetProperties().volume, 1);
}

TEST(Hull, Sphere) {
  Manifold sphere = Manifold::Sphere(1, 6000);
  sphere = sphere.Translate(glm::vec3(0.5));
  Manifold sphereHull = sphere.Hull();
  EXPECT_EQ(sphereHull.NumTri(), sphere.NumTri());
  EXPECT_FLOAT_EQ(sphereHull.GetProperties().volume, 4.1887856);
}
