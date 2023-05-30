#include "Cylinder.h"
#include <cmath>


float Cylinder::intersect(glm::vec3 pos, glm::vec3 dir)
{
    glm::vec3 d = pos - center;
    float a = dir.x * dir.x + dir.z * dir.z;
    float b = 2.0f * (dir.x * d.x + dir.z * d.z);
    float c = d.x * d.x + d.z * d.z - radius * radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant <= 0.0f)
        return -1.0f; // No intersection

    float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
    float t2 = (-b + sqrt(discriminant)) / (2.0f * a);

    float y1 = pos.y + t1 * dir.y;
    float y2 = pos.y + t2 * dir.y;

    if (y1 >= center.y && y1 <= center.y + height) {
        return ((t1 > 0) ? t1 : -1); // Return the positive intersection within the height range
    }
    else if (y2 >= center.y && y2 <= center.y + height) {
        return ((t2 > 0) ? t1 : -1); // Return the positive intersection within the height range
    }
    return -1; // No intersection within the height range
}

glm::vec3 Cylinder::normal(glm::vec3 p)
{
    float maxHeight = center.y + height;
    glm::vec3 pCenter = p - center;
    glm::vec3 normal = glm::vec3(pCenter.x, 0.0, pCenter.z);
    if (p.y >= maxHeight) {
        normal = glm::vec3(0.0, 1.0, 0.0);
    } else if (p.y <= center.y) {
        normal = glm::vec3(0.0, -1.0, 0.0);
    }
    normal = glm::normalize(normal);
    return normal;
}
