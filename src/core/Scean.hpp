#pragma once

#include <vector>

#include "Camera.hpp"

#include <renderer/WorldObject.h>
#include <renderer/Mesh.h>
#include <renderer/Shader.h>
#include <renderer/Light.h>
#include <memory>

class Scean
{

    public:
        Scean(){}
        ~Scean(){}

        inline std::size_t addMesh(std::shared_ptr<Mesh> mesh) 
        { 
            m_meshes.push_back(mesh); 
            return m_meshes.size() - 1;
        }

        inline std::size_t addShader(std::shared_ptr<Shader> shader) 
        { 
            m_shaders.push_back(shader); 
            return m_shaders.size() - 1;
        }

        inline std::size_t addWorldObject(const WorldObject& worldObject) 
        { 
            m_worldObjects.push_back(worldObject); 
            return m_worldObjects.size() - 1;
        }

        inline std::size_t addLight(const Light& light)
        {
            m_lights.push_back(light);
            return m_lights.size() - 1;
        }

        inline Light& getLight(std::size_t index)
        {
            return m_lights.at(index);
        }

        inline const std::vector<Light>& getLights() const
        {
            return m_lights;
        }

        inline std::vector<Light>& getLights()
        {
            return m_lights;
        }
        
        inline void setCamera(std::shared_ptr<Camera> camera) 
        { 
            m_camera = camera; 
        }

        inline std::shared_ptr<Camera> getCamera() const
        {
            return m_camera;
        }

        inline WorldObject& getWorldObject(std::size_t index)
        {
            return m_worldObjects.at(index);
        }

        std::vector<WorldObject>& getWorldObjects()
        {
            return m_worldObjects;
        }

        inline std::shared_ptr<Mesh> getMesh(std::size_t index) const
        {
            return m_meshes.at(index);
        }

        inline std::shared_ptr<Shader> getShader(std::size_t index) const
        {
            return m_shaders.at(index);
        }

        inline void RenderScene() const
        {
            for (const auto& worldObject : m_worldObjects)
            {
                worldObject.draw(m_camera->getProjectionMatrix(),
                                  m_camera->getViewMatrix(),
                                  m_camera->getPosition(),
                                  m_lights);
            }
        }
    
    private:
        std::vector<std::shared_ptr<Mesh>>              m_meshes;
        std::vector<std::shared_ptr<Shader>>            m_shaders;
        std::vector<WorldObject>                        m_worldObjects;
        std::vector<Light>                              m_lights;
        
        std::shared_ptr<Camera>                         m_camera;
};
