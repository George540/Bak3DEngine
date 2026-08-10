#pragma once

#include "renderable_object.h"

class Quad : RenderableObject
{
public:
    Quad();
    ~Quad() override = default;

    void draw() const override;
};
