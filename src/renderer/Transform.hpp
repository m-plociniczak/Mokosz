#pragma once
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
   
    public:
        Transform(glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f))
            : m_position(position), m_rotation(rotation), m_scale(scale) {};

        ~Transform(){};

        inline glm::vec3 position() const { return m_position; }
        inline glm::vec3 rotation() const { return m_rotation; }
        inline glm::vec3 scale() const { return m_scale; }

        inline void setPosition(const glm::vec3& position) { m_position = position; }
        inline void setRotation(const glm::vec3& rotation) { m_rotation = rotation;}
        inline void setScale(const glm::vec3& scale) { m_scale = scale;}                

        inline glm::mat4 getModelMatrix() const
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, m_position);
            model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, m_scale);
            return model;
        }

     private:
        glm::vec3 m_position = glm::vec3(0.0f);
        glm::vec3 m_rotation = glm::vec3(0.0f);
        glm::vec3 m_scale    = glm::vec3(1.0f);
};
