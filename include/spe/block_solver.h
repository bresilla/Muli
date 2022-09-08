#pragma once

#include "common.h"

namespace spe
{

class Contact;
class ContactSolver;
struct Jacobian;

class BlockSolver
{
public:
    void Prepare(Contact* contact);
    void Solve();

private:
    Contact* c;

    ContactSolver* nc1;
    ContactSolver* nc2;

    Jacobian* j1;
    Jacobian* j2;

    Mat2 k;
    Mat2 m;

    void ApplyImpulse(const Vec2& lambda);
};

} // namespace spe