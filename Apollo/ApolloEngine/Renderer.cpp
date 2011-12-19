#include "Renderer.h"

namespace Apollo {

const Image* Renderer::RenderAll(const Scene* scene) {
    const Camera* camera = scene->GetCamera();
    Rect<UINT> region(0, 0, camera->GetImageWidth(), camera->GetImageHeight());
    return Render(scene, region);
}

}
