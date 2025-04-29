![logo](misc/logo.gif)

# Muli

[![Build library](https://github.com/Sopiro/Muli/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/Sopiro/Muli/actions/workflows/cmake-multi-platform.yml)

2D Rigidbody physics engine

## Features  

### Collision  
  - Continuous collision detection (Bilateral advancement by Erin Catto of box2d)
  - Shapes: circle, capsule and convex polygon
  - Support for rounded polygons
  - Multiple colliders attached to single body
  - Dynamic, static and kinematic bodies
  - Collision filtering
  - Dynamic AABB tree broadphase
  - Accelerated raycast, shapecast and area query
  - Easy-to-use collision detection and distance functions
  
### Physics Simulation
  - Continuous physics simulation (Time of impact solver and sub-stepping)  
  - PGS Solver with separate position solver
  - Stable stacking with 2-contact LCP solver (Block solver)
  - Efficient and persistent contact management from box2d
  - Constraint islanding and sleeping
  - Contact callbacks: begin, touching, end, pre-solve, post-solve and destroy
  - Physics material: friction, restitution and surface speed
  - Various joints: angle, distance, grab, line, motor, prismatic, pulley, revolute and weld
  
### Others
  - Cross platform library (C++20)
  - Intuitive and straightforward API design

## Example

``` c++
#include "muli/muli.h"

using namespace muli;

int main()
{
    WorldSettings settings;
    World world(settings);
  
    RigidBody* box = world.CreateBox(1.0f);
    box->SetPosition(0.0f, 5.0f);
  
    // Run simulation for one second
    float dt = 1.0f / 60.0f;
    for(int i = 0; i < 60; ++i)
    {
        world.Step(dt);
    }
  
    return 0;
}
```

## CMALE

``` cmake

include(FetchContent)
fetchcontent_declare(
    muli
    GIT_REPOSITORY https://github.com/bresilla/Muli
)
FetchContent_MakeAvailable(muli)
include_directories(${muli_SOURCE_DIR}/include)

target_link_libraries(your-project PRIVATE muli::muli)
```

## References
Here are some great resources to learn how to build a physics engine!
- https://www.toptal.com/game/video-game-physics-part-i-an-introduction-to-rigid-body-dynamics
- https://allenchou.net/game-physics-series/
- https://box2d.org/publications/
- https://www.cs.cmu.edu/~baraff/sigcourse/index.html
- https://dyn4j.org/blog/
