#include "Cone.h"
#include <cmath>

float Cone::intersect(glm::vec3 pos, glm::vec3 dir)
{
    glm::vec3 dif = pos - center;
    float coneH = height + center.y;
    float r = (radius/height) * (radius / height);
    dif.y = coneH - pos.y;

    float a = dir.x * dir.x + dir.z * dir.z - r * dir.y * dir.y;
    float b = 2 * (dir.x * dif.x + dir.z * dif.z + r * dir.y * dif.y);
    float c = dif.x * dif.x + dif.z * dif.z - dif.y * dif.y * r;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0.001)
    {
        // No intersection
        return -1;
    }
    else
    {
        // Check the two intersection points
        float t1 = (-b - sqrt(discriminant)) / (2 * a);
        float t2 = (-b + sqrt(discriminant)) / (2 * a);

        float s;

        if (t1 >= 0) {
            s = t1;
        } else if (t2 > 0) {
            s = t2;
        } else {
            s = -1;
        }

        return ((pos.y + s * dir.y) > coneH) ? -1 : s;
    }
}

glm::vec3 Cone::normal(glm::vec3 pos)
{
    glm::vec3 pCenter = pos - center;
    float alpha = atan(pCenter.x/pCenter.z);
    float theta = atan(radius/height);

    glm::vec3 normal = glm::vec3(sin(alpha)*cos(theta), sin(theta), cos(alpha)*cos(theta));
    return normal;
}
