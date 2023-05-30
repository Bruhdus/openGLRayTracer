#ifndef CONE_H
#define CONE_H

#include <glm/glm.hpp>
#include "SceneObject.h"

/**
 * Defines a simple Cone located at 'center'
 * with the specified height and radius
 */
class Cone : public SceneObject {
private:
    glm::vec3 center = glm::vec3(0);
    float height = 1;
    float radius = 1;

public:
    Cone() {};  // Default constructor creates a unit cone

    Cone(glm::vec3 c, float h, float r) : center(c), height(h), radius(r) {}

    float intersect(glm::vec3 p0, glm::vec3 dir);

    glm::vec3 normal(glm::vec3 p);
};
#endif // CONE_H
