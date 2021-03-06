#pragma once

#include "ApolloCommon.h"
#include "ApolloAllocator.h"
#include "Model.h"
#include "Camera.h"
#include "Accel.h"
#include "Light.h"

//
// Represent all of the objects, lights, materials, camera to represent one scene.
// Every scene generates one rendered frame.
//

namespace Apollo {
    class Scene {
    public:
        Scene(const std::string& name="");
        ~Scene();

    public:
        // Precomputes before rendering the scene.  Nothing that changes the scene should be set after initialize
        void Initialize();

    // Functions to set the objects in the scene
    public:
        // Sets the camera
        void SetCamera(Camera* camera) { m_camera = camera; m_cameraDirty = true;}

        // Add a model to the scene
        void AddModel(Model* m) { m_models.push_back(m); m_geometryDirty = true;}

		void AddModels(const std::vector<Model*>& models) {
			m_models.insert(m_models.end(), models.begin(), models.end());
			m_geometryDirty = true;
		}
        
        // Add a light to the scene
        void AddLight(Light* light);
		void AddLights(const std::vector<Light*>& lights);

        // Sets the accelerator data structure
        void SetAccel(Accel* accel) { m_accel = accel; m_geometryDirty = true;}

		// Sets the environment shader for rays that do not hit anything
		void SetEnvironmentShader(Shader* s) { m_env_shader = s; }

    // Functions to access objects
    public:
        PoolAllocator* GetAllocator() { return &m_allocator; }
        Camera* GetCamera() const { return m_camera; }
        Accel* GetAccel() const { return m_accel; }
		Shader* GetEnvironmentShader() const { return m_env_shader; }
        const Color4f& GetAmbientLight() const { return m_ambientLight; }
        const std::vector<Light*>& GetLights() const { return m_lights; }

    private:
        PoolAllocator m_allocator;
        Color4f m_ambientLight;
        std::string m_name;
        Camera* m_camera;
        Accel* m_accel;
        std::vector<Model*> m_models;
        std::vector<Light*> m_lights;
		Shader* m_env_shader;

        bool m_cameraDirty;
        bool m_geometryDirty;
        bool m_lightDirty;
    };
}
