/*
 * Abstraction over rendering algorithms.
 */

#pragma once

#include "ApolloCommon.h"
#include "Scene.h"
#include "Camera.h"

namespace Apollo {
    class Renderer {
    public:
        // Render the entire scene
        const Image* RenderAll(const Scene* scene);
        virtual const Image* Render(const Scene* scene, const Rect<UINT>& region) = 0;

    public:
        // Render settings

    public:
        virtual ~Renderer() {}

    protected:
        Renderer(const std::string& name) : m_name (name) {}
        std::string m_name;
    };
}
