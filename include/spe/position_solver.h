#pragma once

#include "common.h"

namespace spe
{

class Contact;

class PositionSolver
{
    friend class Contact;
    friend class BlockSolver;

public:
    void Prepare(Contact* _contact, uint32_t _index);
    void Solve();

private:
    Contact* contact;

    Vec2 localPlainPoint;
    Vec2 localClipPoint;
    Vec2 localNormal;
};

} // namespace spe
