#pragma once

#include <glad/glad.h>

/*
 * RAII wrapper around GL_KHR_debug's push/pop debug group commands.
 *
 * Usage:
 *   {
 *       DebugScopeGroup scope("Base Geometry Pass");
 *       model->draw();
 *   }
 */
class DebugScopeGroup
{
public:
    explicit DebugScopeGroup(const char* label) { glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, label); }
    ~DebugScopeGroup() { glPopDebugGroup(); }

    DebugScopeGroup(const DebugScopeGroup&) = delete;
    DebugScopeGroup& operator=(const DebugScopeGroup&) = delete;
};
