/*==================================================================================
* COSC 363  Computer Graphics (2023)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab06.pdf   for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Cylinder.h"
#include "Cone.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include <GL/freeglut.h>
#include "TextureBMP.h"
using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
bool antiAliasingOn = true;
bool fogOn = true;

vector<SceneObject*> sceneObjects;
TextureBMP texture;


//---The most important function in a ray tracer! ----------------------------------
//   Computes the colour value obtained by tracing a ray and finding its
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
    glm::vec3 fogColor(1,1,1);
    glm::vec3 backgroundCol(0);                     //Background colour = (0,0,0)
    glm::vec3 lightPos(30, 39.8, -150);                 //Light's position
    glm::vec3 color(0);
    SceneObject* obj;

    ray.closestPt(sceneObjects);                    //Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;       //no intersection
    obj = sceneObjects[ray.index];                  //object on which the closest point of intersection is found
    if (ray.index == 4) {
        // Checkerboard pattern
        int checkSize = 5; // Size of each square in the checkerboard
        int ix = (ray.hit.x + 40) / checkSize;
        int iz = ray.hit.z / checkSize;
        int k = (abs(ix) + iz) % 2; // Alternate between two colors based on the sum of ix and iz

        if (k == 0)
            color = glm::vec3(0.6, 0.4, 1); // First color purple
        else
            color = glm::vec3(1, 1, 0.5); // Second color yellow
        obj->setColor(color);
    }

    // Texturing the 4th sphere
    if (ray.index == 3) {
        glm::vec3 normal = obj->normal(ray.hit);
        float texcoords = 0.5 + (atan2(normal.x, normal.z) / (2 * M_PI));
        float texcoordt = 0.5 + (asin(normal.y) / M_PI);
        if(texcoords > 0 && texcoords < 1 && texcoordt > 0 && texcoordt < 1) {
            color = texture.getColorAt(texcoords, texcoordt);
            obj->setColor(color);
        }
    }




    color = obj->lighting(lightPos, -ray.dir, ray.hit);     //Object's colour
    glm::vec3 lightVec = lightPos - ray.hit;
    Ray shadowRay(ray.hit, lightVec);
    shadowRay.closestPt(sceneObjects);
    if (shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec)) {
        SceneObject* object = sceneObjects[shadowRay.index];
        if (object->isTransparent() || object->isRefractive()) {
            color = 0.6f * color + object->getColor() * 0.2f;
        } else {
            color = 0.2f * color;
        }
    }

    if (obj->isReflective() && step < MAX_STEPS) {
        float rho = obj->getReflectionCoeff();
        glm::vec3 normalVec = obj->normal(ray.hit);
        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
        Ray reflectedRay(ray.hit, reflectedDir);
        glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
        color = color + (rho * reflectedColor);
    }

    if (obj->isRefractive() && step < MAX_STEPS) {
        float eta = obj->getRefractiveIndex();
        glm::vec3 normalVec = obj->normal(ray.hit);
        glm::vec3 g = glm::refract(ray.dir, normalVec, 1/eta);
        Ray refrRay(ray.hit, g);
        refrRay.closestPt(sceneObjects);
        glm::vec3 m = obj->normal(refrRay.hit);
        glm::vec3 h = glm::refract(g, -m, eta/1);
        Ray outRay(refrRay.hit, h);
        glm::vec3 reflectedColor = trace(outRay, step + 1);
        float rho = obj->getRefractionCoeff();
        color = (reflectedColor * rho) + ((1- rho) * color);
    }

    if (obj->isTransparent() && step < MAX_STEPS) {
        float transCo = obj->getTransparencyCoeff();
        Ray internalRay(ray.hit, ray.dir);
        internalRay.closestPt(sceneObjects);
        glm::vec3 newColor = trace(internalRay, step + 1);
        color = (color * 0.3f) + (transCo * newColor);
    }

    if (fogOn) {
        float lambda = (ray.hit.z + 200.0f) / -90.0f;
        lambda = glm::clamp(lambda, 0.0f, 1.0f);
        color = ((1-lambda) * color) + (lambda * fogColor);
    }

    return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
    float xp, yp;  //grid point
    float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
    float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
    glm::vec3 eye(0., 0., -20.);

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);  //Each cell is a tiny quad.

    for (int i = 0; i < NUMDIV; i++)    //Scan every cell of the image plane
    {
        xp = XMIN + i * cellX;
        for (int j = 0; j < NUMDIV; j++)
        {
            yp = YMIN + j * cellY;

            glm::vec3 col;
            if (antiAliasingOn) {
                float left = xp + 0.25 * cellX;
                float right = xp + 0.75 * cellX;
                float bottom = yp + 0.25 * cellY;
                float top = yp + 0.75 * cellY;

                glm::vec3 dir0(left, bottom, -EDIST);
                glm::vec3 dir1(left, bottom, -EDIST);
                glm::vec3 dir2(right, top, -EDIST);
                glm::vec3 dir3(right, top, -EDIST);

                Ray ray = Ray(eye, dir0);
                col = trace(ray, 1);

                ray = Ray(eye, dir1);
                col += trace(ray, 1);

                ray = Ray(eye, dir2);
                col += trace(ray, 1);

                ray = Ray(eye, dir3);
                col += trace(ray, 1);

                col = col / 4.0f;
            } else {
                glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);  //direction of the primary ray
                Ray ray = Ray(eye, dir);
                col = trace(ray, 1); //Trace the primary ray and get the colour value
            }

            glColor3f(col.r, col.g, col.b);
            glVertex2f(xp, yp);             //Draw each cell with its color value
            glVertex2f(xp + cellX, yp);
            glVertex2f(xp + cellX, yp + cellY);
            glVertex2f(xp, yp + cellY);
        }
    }

    glEnd();
    glFlush();
}



//---This function initializes the scene -------------------------------------------
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

    // Reflective Sphere
    Sphere *sphere1 = new Sphere(glm::vec3(-4, -8, -130), 7);
    sphere1->setColor(glm::vec3(0.3, 0, 0.3));
    sceneObjects.push_back(sphere1);
    sphere1->setReflectivity(true, 0.8);

    // Refractive Sphere
    Sphere *sphere2 = new Sphere(glm::vec3(10, 0, -110), 6);
    sphere2->setColor(glm::vec3(0.3, 0.3, 0.3));
    sceneObjects.push_back(sphere2);
    sphere2->setRefractivity(true, 0.8, 1.2);

    // Transparent Sphere
    Sphere *sphere3 = new Sphere(glm::vec3(14, 10, -110), 5);
    sphere3->setColor(glm::vec3(0, 0, 1));
    sceneObjects.push_back(sphere3);
    sphere3->setTransparency(true, 0.9);
    sphere3->setReflectivity(true, 0.05);

    // Textured Sphere
    Sphere *sphere4 = new Sphere(glm::vec3(-18, 3, -180), 9);
    sceneObjects.push_back(sphere4);

    Plane *planeFloor = new Plane (glm::vec3(-40, -15, 0), //Point A
                                glm::vec3(40, -15, 0), //Point B
                                glm::vec3(40, -15, -300), //Point C
                                glm::vec3(-40, -15, -300)); //Point D
    planeFloor->setColor(glm::vec3(1, 1, 0.2));
    planeFloor->setSpecularity(false);
    sceneObjects.push_back(planeFloor);

    Plane *planeCeiling = new Plane (glm::vec3(-40, 40, 0), //Point A
                                glm::vec3(40, 40, 0), //Point B
                                glm::vec3(40, 40, -300), //Point C
                                glm::vec3(-40, 40, -300)); //Point D
    planeCeiling->setColor(glm::vec3(1, 1, 1));
    planeCeiling->setSpecularity(false);
    sceneObjects.push_back(planeCeiling);

    Plane *planeFrontWall = new Plane (glm::vec3(-40, -15, -300), //Point A
                                glm::vec3(40, -15, -300), //Point B
                                glm::vec3(40, 40, -300), //Point C
                                glm::vec3(-40, 40, -300)); //Point D
    planeFrontWall->setColor(glm::vec3(1, 0.2, 0.2));
    planeFrontWall->setSpecularity(false);
    sceneObjects.push_back(planeFrontWall);

    Plane *planeBackWall = new Plane (glm::vec3(-40, -15, 0), //Point A
                                glm::vec3(40, -15, 0), //Point B
                                glm::vec3(40, 40, 0), //Point C
                                glm::vec3(-40, 40, 0)); //Point D
    planeBackWall->setColor(glm::vec3(0.2, 0.2, 0.2));
    planeBackWall->setSpecularity(false);
    sceneObjects.push_back(planeBackWall);

    Plane *planeRightWall = new Plane (glm::vec3(40, -15, -300), //Point A
                                glm::vec3(40, -15, 0), //Point B
                                glm::vec3(40, 40, 0), //Point C
                                glm::vec3(40, 40, -300)); //Point D
    planeRightWall->setColor(glm::vec3(0.2, 1, 0.2));
    planeRightWall->setSpecularity(false);
    sceneObjects.push_back(planeRightWall);

    Plane *planeLeftWall = new Plane (glm::vec3(-40, -15, 0), //Point A
                                glm::vec3(-40, -15, -300), //Point B
                                glm::vec3(-40, 40, -300), //Point C
                                glm::vec3(-40, 40, 0)); //Point D
    planeLeftWall->setColor(glm::vec3(0.2, 0.2, 1));
    planeLeftWall->setSpecularity(false);
    sceneObjects.push_back(planeLeftWall);


    Plane *mirror = new Plane (glm::vec3(-25, -8, -200), //Point A
                                glm::vec3(25, -8, -200), //Point B
                                glm::vec3(25, 25, -200), //Point C
                                glm::vec3(-25, 25, -200)); //Point D
    mirror->setColor(glm::vec3(0, 0, 0));
    sceneObjects.push_back(mirror);
    mirror->setReflectivity(true, 1);

    Plane *backMirror = new Plane (glm::vec3(-25, -8, -1), //Point A
                                glm::vec3(25, -8, -1), //Point B
                                glm::vec3(25, 25, -1), //Point C
                                glm::vec3(-25, 25, -1)); //Point D
    backMirror->setColor(glm::vec3(0, 0, 0));
    sceneObjects.push_back(backMirror);
    backMirror->setReflectivity(true, 1);

    Cylinder *cylinder = new Cylinder(glm::vec3(10, -15, -110), 3, 9);
    cylinder->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(cylinder);

    Cone *cone = new Cone(glm::vec3(-18, 0, -180), 1, 0.5);
    cone->setColor(glm::vec3(1, 0.2, 0.6));
    sceneObjects.push_back(cone);

    texture = TextureBMP("Earth.bmp");
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
