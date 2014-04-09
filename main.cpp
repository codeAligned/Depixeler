#include <iostream>
#include <cassert>
#include <vector>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include "data.h"
#include "matrix.h"
#include "readpng.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGl/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

const double RAD_TO_DEG = 180/M_PI;

static int lastX, lastY;
static int downX, downY;

// to remove: vec.erase(vec.begin() + index);
int xRes, yRes;

int userTransformType;

int drawingType;

int gWidth;
int gHeight;
unsigned char *gData;
int gDrawMode = 1;

vector <Vertex> DualVertices;
vector <DualEdge> DualEdges;
vector <vector <Vertex> > DualFaces;
vector <vector <int> > DualFacesIndices;

vector <Vertex> Particles;

int currentSelection = -1;
int currentDualFace = -1;
int gDebug = 0;

/** PROTOTYPES **/
void redraw();
void initGL();
void resize(GLint w, GLint h);
void keyfunc(GLubyte key, GLint x, GLint y);


// given a value of the pixel that we clicked on, return the value in
// object space (which is on a scale from -1 to 1). 
float getNDCCoord(int pixel, int min, int max, int resolution) 
{

    float smallScale = max - min;
    float result = smallScale * (pixel - resolution/smallScale) / resolution;

    //std::cout << "getNDCCoord " << result << std::endl;
    return result;
}

double fRand(double fMin, double fMax) {
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

bool isLeft(Vec3 a, Vec3 b, Vec3 c){
    return (((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) >= 0);
}

int findTriangle(double x, double y) 
{
    /*
    Vertex point(x,y);
    for (size_t i = 0; i < PrimalTriangles.size(); i++) {
        const Triangle &triangle= PrimalTriangles[i];
        if (triangle.isPointInside(point)) {
            return (int)i;
        }
    }
    */
    std::cout << "COULD NOT FIND" << std::endl;
    return -1;
}

double signedArea(Vertex &v1, Vertex &v2, Vertex &v3) {

    double term1 = (v1.x * v2.y) - (v1.x * v3.y); // x1*y2 - x1*y3
    double term2 = (v3.x * v1.y) - (v2.x * v1.y); // x3*y1 - x2*y1
    double term3 = (v2.x * v3.y) - (v3.x * v2.y); // x2*y3 - x3*y2

    double area = 0.5 * (term1 + term2 + term3);
    return area;
}

double distance(double x1, double y1, double x2, double y2) {
    double xSq = (x2 - x1) * (x2 - x1);
    double ySq = (y2 - y1) * (y2 - y1);
    double d = sqrt(xSq + ySq);
    return d;
}

int getUpNeighbor(int x, int y, int imageWidth, int imageHeight) {
    // assume gData stores RGB of each pixel

    if (y == imageHeight - 1) {
        return -1;
    }

    int newY = y + 1;
    int index = 3 * x + (3 * imageWidth * newY);

    //std::cout << "(" << x << "," << y << ")  " << int(gData[index]) << " " <<  int(gData[index+1]) << " " << int(gData[index+2]) << std::endl;
    //std::cout << "up ind: " << index << std::endl;
    //std::cout << "y: " << y << std::endl;


    return index;
}

int getDownNeighbor(int x, int y, int imageWidth, int imageHeight) {
    // assume gData stores RGB of each pixel

    if (y == 0) {
        return -1;
    }

    int newY = y - 1;
    int index = 3 * x + (3 * imageWidth * newY);

    //std::cout << "(" << x << "," << y << ")  " << int(gData[index]) << " " <<  int(gData[index+1]) << " " << int(gData[index+2]) << std::endl;
    //std::cout << "down ind: " << index << std::endl;
    //std::cout << "y: " << y << std::endl;


    return index;
}

int getLeftNeighbor(int x, int y, int imageWidth, int imageHeight) {
    // assume gData stores RGB of each pixel

    if (x == 0) {
        return -1;
    }

    int newX = x - 1;
    int index = 3 * newX + (3 * imageWidth * y);

    //std::cout << "(" << x << "," << y << ")  " << int(gData[index]) << " " <<  int(gData[index+1]) << " " << int(gData[index+2]) << std::endl;
    //std::cout << "left ind: " << index << std::endl;
    //std::cout << "y: " << y << std::endl;


    return index;
}

int getRightNeighbor(int x, int y, int imageWidth, int imageHeight) {
    // assume gData stores RGB of each pixel

    if (x == imageWidth - 1) {
        return -1;
    }

    int newX = x + 1;
    int index = 3 * newX + (3 * imageWidth * y);

    //std::cout << "(" << x << "," << y << ")  " << int(gData[index]) << " " <<  int(gData[index+1]) << " " << int(gData[index+2]) << std::endl;
    //std::cout << "right ind: " << index << std::endl;
    //std::cout << "y: " << y << std::endl;


    return index;
}



void drawPNG() {
    double h = gHeight;
    double w = gWidth;

    double max;
    if (gHeight > gWidth) {
        max = gHeight;
    } else {
        max = gWidth;
    }

    /*
     * This is for pressing the key '2'. It renders the png image as it is.
     * It can be blown up to whatever size the user desires, and the pixels/pixel 
     * size will be scaled accordingly with no algorithm attempted to depixel the
     * png image.
     *
     */
    if (gDrawMode == 1) {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        double pointSize = int(yRes/h) + 1.0;

        glPointSize(pointSize);
        glBegin(GL_POINTS);
        glVertex3f(0.95,-0.95,0);

        for(int y=0; y<h; y++) {
            for (int x = 0; x < 3*w; x += 3) {
                int index = x + (3 * w * y);

                glColor3d(int(gData[index]) / 256.0, int(gData[index+1]) / 256.0, int(gData[index+2]) / 256.0);
                glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max, -1 + (pointSize / yRes)+ 2*y/max, 0);
            }
        }
        glEnd();
    }

   else if (gDrawMode == 2) {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glColor3f(0.0f, 0.0f, 0.0f);
        //GLubyte color[2][2][4];

        
        GLubyte color[gHeight][gWidth][4];
        for(int y = 0; y < gHeight; y++) {
            for (int x = 0; x < 3 * gWidth; x += 3) {
                int index = x + (3 * w * y);

                color[y][x/3][0] = int(gData[index]);
                color[y][x/3][1] = int(gData[index+1]);
                color[y][x/3][2] = int(gData[index+2]);
                color[y][x/3][3] = 0;

                //std::cout << "X: " << x/3 << " Y: " << y << "  rgb: " << int(gData[index]) << " " << int(gData[index + 1]) << " " << int(gData[index+2]) << std::endl;

                //glColor3d(int(gData[index]) / 256.0, int(gData[index+1]) / 256.0, int(gData[index+2]) / 256.0);
                //glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max, -1 + (pointSize / yRes)+ 2*y/max, 0);
            }
        }
        

        /*

        // red

        color[0][0][0] = 255;

        color[0][0][1] = 0;

        color[0][0][2] = 0;
        color[0][0][3] = 0;


        // green
        color[0][1][0] = 0;
        color[0][1][1] = 255;

        color[0][1][2] = 0;
        color[0][1][3] = 0;


        // blue
        color[1][0][0] = 0;
        color[1][0][1] = 0;

        color[1][0][2] = 255;

        color[1][0][3] = 0;


        // white

        color[1][1][0] = 255;
        color[1][1][1] = 255;
        color[1][1][2] = 255;

        color[1][1][3] = 0;
        */
        
 
        //GLfloat scaleX = 1.0f * xRes / 2;
        //GLfloat scaleY = 1.0f * yRes / 2;
        //std::cout << scaleX << " " << scaleY << std::endl;
        //glPixelZoom(scaleX/15.99, scaleY/15.99);
        glPixelZoom(floor(xRes/max), floor(yRes/max));

        //glPixelZoom(5,5);
        glRasterPos2d(-1.0, -1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels( gWidth, gHeight, GL_RGBA, GL_UNSIGNED_BYTE, color );
        glFlush();
    }
    
    /*
     * This is for pressing the key '3'. It renders the png image using the EPX algorithm. Basically, 
     * we break down each pixel into a 2x2 (so 4 pixels). If two same-color pixels of the original 
     * png image are adjacent to a pixel of the original png image such that the three pixels form 
     * a 90 degree angle, then the corresponding pixel in the blown up 2x2 is that color as well. So,
     * for example, let's say that pixel A and pixel B are the same color white such that pixel A 
     * is adjacently above pixel C and pixel B is adjacently to the right of pixel C. We then blow up
     * pixel C into a 2x2 set of pixels. Since pixels A and B are above and right of pixel C, we would 
     * then render the top right pixel of the 2x2 with the same color, white. This is essentially the 
     * EPX algorithm.
     *
     */ 

    else if (gDrawMode == 3) {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        double pointSize = round(0.5 * yRes/h + 0.5);

        glPointSize(pointSize);
        glBegin(GL_POINTS);
        glVertex3f(0.95,-0.95,0);

        for(int y=0; y<h; y++) {
            for (int x = 0; x < 3*w; x += 3) {
                int index = x + (3 * w * y);

                glColor3d(int(gData[index]) / 256.0, int(gData[index+1]) / 256.0, int(gData[index+2]) / 256.0);

                int upIndex = getUpNeighbor(x/3,y, gWidth, gHeight);
                int downIndex = getDownNeighbor(x/3,y, gWidth, gHeight);
                int leftIndex = getLeftNeighbor(x/3,y, gWidth, gHeight);
                int rightIndex = getRightNeighbor(x/3,y, gWidth, gHeight);

                // this is for the corner cases
                if ((upIndex == -1 && leftIndex == -1) ||
                    (upIndex == -1 && rightIndex == -1) ||
                    (downIndex == -1 && leftIndex == -1) ||
                    (downIndex == -1 && rightIndex == -1))
                {
                    // bottom-left pixel of the 2x2
                    glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max, -1 + (pointSize / yRes)+ 2*y/max, 0);
                    // bottom-right pixel of the 2x2
                    glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max + 2*pointSize/xRes, -1 + (pointSize / yRes)+ 2*y/max , 0);
                    // top-left pixel of the 2x2
                    glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max, -1 + (pointSize / yRes)+ 2*y/max + 2*pointSize/yRes, 0);
                    // top-right pixel of the 2x2
                    glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max + 2*pointSize/xRes, -1 + (pointSize / yRes)+ 2*y/max + 2*pointSize/yRes, 0);                    
                }
                
                else
                {
                    Vec3 up;
                    Vec3 down;
                    Vec3 right;
                    Vec3 left;
                    up.x = gData[upIndex];
                    up.y = gData[upIndex+1];
                    up.z = gData[upIndex+2];
                    down.x = gData[downIndex];
                    down.y = gData[downIndex+1];
                    down.z = gData[downIndex+2];
                    right.x = gData[rightIndex];
                    right.y = gData[rightIndex+1];
                    right.z = gData[rightIndex+2];
                    left.x = gData[leftIndex];
                    left.y = gData[leftIndex+1];
                    left.z = gData[leftIndex+2];

                    bool upLeft = (up == left);
                    bool upRight = (up == right);
                    bool downLeft = (down == left);
                    bool downRight = (down == right);
                    //std::cout << upLeft << std::endl;
                    //std::cout << upRight << std::endl;
                    //std::cout << downLeft << std::endl;
                    //std::cout << downRight << std::endl;

                    if (downLeft) {
                        glColor3d(down.x / 256.0, down.y / 256.0, down.z / 256.0);
                    } 
                    // bottom-left pixel of the 2x2
                    glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max, -1 + (pointSize / yRes)+ 2*y/max, 0);
                    glColor3d(int(gData[index]) / 256.0, int(gData[index+1]) / 256.0, int(gData[index+2]) / 256.0);

                    if (downRight) {
                        glColor3d(down.x / 256.0, down.y / 256.0, down.z / 256.0);
                    } 
                    // bottom-right pixel of the 2x2
                    glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max + 2*pointSize/xRes, -1 + (pointSize / yRes)+ 2*y/max , 0);
                    glColor3d(int(gData[index]) / 256.0, int(gData[index+1]) / 256.0, int(gData[index+2]) / 256.0);

                    if (upLeft) {
                        glColor3d(up.x / 256.0, up.y / 256.0, up.z / 256.0);
                    }
                    // top-left pixel of the 2x2
                    glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max, -1 + (pointSize / yRes)+ 2*y/max + 2*pointSize/yRes, 0);
                    glColor3d(int(gData[index]) / 256.0, int(gData[index+1]) / 256.0, int(gData[index+2]) / 256.0);

                    if (upRight) {
                        glColor3d(up.x / 256.0, up.y / 256.0, up.z / 256.0);
                    }
                    // top-right pixel of the 2x2
                    glVertex3f(-1 + (pointSize / xRes) + 2*(x/3)/max + 2*pointSize/xRes, -1 + (pointSize / yRes)+ 2*y/max + 2*pointSize/yRes, 0);
                    glColor3d(int(gData[index]) / 256.0, int(gData[index+1]) / 256.0, int(gData[index+2]) / 256.0);

                }
            }
        }
        glEnd();
    }

    /* EPX algorithm with gldrawPixels*/
    else if (gDrawMode == 4) {

        GLubyte color[2*gHeight][2*gWidth][4];

        glClearColor(0.0, 0.0, 0.0, 0.0);
        glVertex3f(0.95,-0.95,0);

        //GLubyte color[gHeight][gWidth][4];
        for(int y = 0; y < gHeight; y++) {
            for (int x = 0; x < 3 * gWidth; x += 3) {
                int index = x + (3 * w * y);

                color[2*y][2*x/3][0] = int(gData[index]);
                color[2*y][2*x/3][1] = int(gData[index + 1]);
                color[2*y][2*x/3][2] = int(gData[index + 2]);
                color[2*y][2*x/3][3] = 0;

                color[2*y][2*x/3 + 1][0] = int(gData[index]);
                color[2*y][2*x/3 + 1][1] = int(gData[index + 1]);
                color[2*y][2*x/3 + 1][2] = int(gData[index + 2]);
                color[2*y][2*x/3 + 1][3] = 0;

                color[2*y + 1][2*x/3][0] = int(gData[index]);
                color[2*y + 1][2*x/3][1] = int(gData[index + 1]);
                color[2*y + 1][2*x/3][2] = int(gData[index + 2]);
                color[2*y + 1][2*x/3][3] = 0;

                color[2*y + 1][2*x/3 + 1][0] = int(gData[index]);
                color[2*y + 1][2*x/3 + 1][1] = int(gData[index + 1]);
                color[2*y + 1][2*x/3 + 1][2] = int(gData[index + 2]);
                color[2*y + 1][2*x/3 + 1][3] = 0;


                int upIndex = getUpNeighbor(x/3,y, gWidth, gHeight);
                int downIndex = getDownNeighbor(x/3,y, gWidth, gHeight);
                int leftIndex = getLeftNeighbor(x/3,y, gWidth, gHeight);
                int rightIndex = getRightNeighbor(x/3,y, gWidth, gHeight);

                // this is for the corner cases
                if ((upIndex == -1 && leftIndex == -1) ||
                    (upIndex == -1 && rightIndex == -1) ||
                    (downIndex == -1 && leftIndex == -1) ||
                    (downIndex == -1 && rightIndex == -1))
                {
                    color[2*y][2*x/3][0] = int(gData[index]);
                    color[2*y][2*x/3][1] = int(gData[index + 1]);
                    color[2*y][2*x/3][2] = int(gData[index + 2]);
                    color[2*y][2*x/3][3] = 0;
                }
                
                else
                {
                    
                    Vec3 up;
                    Vec3 down;
                    Vec3 right;
                    Vec3 left;
                    up.x = gData[upIndex];
                    up.y = gData[upIndex+1];
                    up.z = gData[upIndex+2];
                    down.x = gData[downIndex];
                    down.y = gData[downIndex+1];
                    down.z = gData[downIndex+2];
                    right.x = gData[rightIndex];
                    right.y = gData[rightIndex+1];
                    right.z = gData[rightIndex+2];
                    left.x = gData[leftIndex];
                    left.y = gData[leftIndex+1];
                    left.z = gData[leftIndex+2];


                    bool upLeft = (up == left);
                    bool upRight = (up == right);

                    bool downLeft = (down == left);
                    bool downRight = (down == right);

                    //std::cout << upLeft << std::endl;
                    //std::cout << upRight << std::endl;
                    //std::cout << downLeft << std::endl;
                    //std::cout << downRight << std::endl;


                    if (downLeft) {

                        color[2*y][2*x/3][0] = int(down.x);
                        color[2*y][2*x/3][1] = int(down.y);

                        color[2*y][2*x/3][2] = int(down.z);
                        color[2*y][2*x/3][3] = 0;

                    } 

                    if (downRight) {

                        color[2*y][2*x/3 + 1][0] = int(down.x);
                        color[2*y][2*x/3 + 1][1] = int(down.y);

                        color[2*y][2*x/3 + 1][2] = int(down.z);
                        color[2*y][2*x/3 + 1][3] = 0;

                    } 

                    if (upLeft) {
                        color[2*y + 1][2*x/3][0] = int(up.x);
                        color[2*y + 1][2*x/3][1] = int(up.y);

                        color[2*y + 1][2*x/3][2] = int(up.z);
                        color[2*y + 1][2*x/3][3] = 0;
                    }


                    if (upRight) {

                        color[2*y + 1][2*x/3 + 1][0] = int(up.x);
                        color[2*y + 1][2*x/3 + 1][1] = int(up.y);

                        color[2*y + 1][2*x/3 + 1][2] = int(up.z);
                        color[2*y + 1][2*x/3 + 1][3] = 0;

                    } 
                        
            
                }
            }
        }
        glPixelZoom(floor(xRes/max)/2, floor(yRes/max)/2);

        //glPixelZoom(5,5);

        glRasterPos2d(-1.0, -1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels( 2*gWidth, 2*gHeight, GL_RGBA, GL_UNSIGNED_BYTE, color );
        glFlush();
    }
 
}

/* Draws the current object, based on gMaterial propereries and the current
 * gSeperator block.
*/
void drawObject() 
{

    glPushMatrix();  // save the current matrix
    
    glDisable(GL_LIGHTING);

    drawPNG();

    /*
    if (currentSelection != -1) { // draw selected triangle (if any)
        Triangle &triangle = PrimalTriangles[currentSelection];

        const Vertex &vertA = triangle.mVert1;
        const Vertex &vertB = triangle.mVert2;
        const Vertex &vertC = triangle.mVert3;
        glColor3d(1,1,0); 
        glBegin(GL_TRIANGLES);  // drawing triangle meshes
        glVertex2d(vertA.x, vertA.y);
        glVertex2d(vertB.x, vertB.y);
        glVertex2d(vertC.x, vertC.y);
        glEnd();

        if (currentDualFace != -1) {
            const vector <Vertex> &faceIndices = DualFaces[currentDualFace];
            glColor3d(0,1,0);
            glBegin(GL_POLYGON);
            for (size_t j = 0; j < faceIndices.size(); j++) {
                const Vertex &vertex = faceIndices[j];
                glVertex2d(vertex.x, vertex.y);
            }
            glEnd();
        }        
    }
    */
    
    glEnable(GL_LIGHTING);
    
    glPopMatrix();  // restore old matrix.

}


/** GLUT callback functions **/

/*
 * This function gets called every time the window needs to be updated
 * i.e. after being hidden by another window and brought back into view,
 * or when the window's been resized.
 * You should never call this directly, but use glutPostRedisply() to tell
 * GLUT to do it instead.
 */
void redraw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawObject();
    glutSwapBuffers();
}


/**
 * GLUT calls this function when the window is resized.
 * All we do here is change the OpenGL viewport so it will always draw in the
 * largest square that can fit in our window..
 */
void resize(GLint w, GLint h)
{
    if (h == 0)
        h = 1;

    // ensure that we are always square (even if whole window not used)
    if (w > h)
        w = h;
    else
        h = w;

    // Reset the current viewport and perspective transformation
    glViewport(0, 0, w, h);
    xRes = w;
    yRes = h;

    // Tell GLUT to call redraw()
    glutPostRedisplay();
}

/*
 * GLUT calls this function when any key is pressed while our window has
 * focus.  Here, we just quit if any appropriate key is pressed.  You can
 * do a lot more cool stuff with this here.
 */
void keyfunc(GLubyte key, GLint x, GLint y)
{
    if (key == 'f') {
        glShadeModel(GL_FLAT);  // set to Flat shading
        drawingType = 1;  // set up to draw actual triangles
        glEnable(GL_LIGHTING);
        glutPostRedisplay();
    } else if (key == 'g') {
        glShadeModel(GL_SMOOTH);  // set to Gouraud shading
        drawingType = 1; // set up to draw actual triangles
        glEnable(GL_LIGHTING);
        glutPostRedisplay();
    } else if (key == 'w') {
        drawingType = 0;  // set up for wireframe
        glDisable(GL_LIGHTING); // turn off lighting for wireframe
        glutPostRedisplay();

    } else if (key == 27 || key == 'q' || key =='Q') {
        exit(0); // escape or q or Q to exit the program
    } else if (key == '1') {
        std::cout << "1" << std::endl;
        gDrawMode = 1;
        glutPostRedisplay();
    } else if (key == '2') {
        std::cout << "2" << std::endl;
        gDrawMode = 2;
        glutPostRedisplay();
    } else if (key == '3') {
        std::cout << "3" << std::endl;
        gDrawMode = 3;
        glutPostRedisplay();
    } else if (key == '4') {
        std::cout << "4" << std::endl;
        gDrawMode = 4;
        glutPostRedisplay();
    }
}


// call this to see if there's any GL error, if there is,
// print out the error
void checkGLError()
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "glError " << gluErrorString(error) << std::endl;
    }
}

void readTex(const std::string &fileName) {

    std::string filePath = fileName;

    int w, h;
    png_bytepp p = readpng(filePath.c_str(), &w, &h);

    gData = new unsigned char [w * h * 3];

    for(int y=0; y<h; y++) {
        png_bytep r = p[h-1-y]; // get the row
        for (int x = 0; x < 3*w; x += 3) {
            int index = x + (3 * w * y);
            gData[index] = r[x];
            gData[index + 1] = r[x+1];
            gData[index + 2] = r[x+2];
        }
    }

    std::cout << "READ" << std::endl;

    gWidth = w;
    gHeight = h;
}

/**
 * Set up OpenGL state.  This does everything so when we draw we only need to
 * actually draw the sphere, and OpenGL remembers all of our other settings.
 */
void initGL()
{
    // Tell openGL to use gouraud shading:
    glShadeModel(GL_SMOOTH);
    drawingType = 1;  // start at Gouraud setting
    
    // Enable back-face culling:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enable depth-buffer test.
    glEnable(GL_DEPTH_TEST);
    
    // Set up projection and modelview matrices ("camera" settings) 
    // Look up these functions to see what they're doing.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    checkGLError();

    // this is the only operation we are doing on GL_PROJECTION matrix

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

static void mouseButton(int button, int state, int x, int y) 
{
    double xNDC = getNDCCoord(x, -1, 1, xRes);
    double yNDC = getNDCCoord(y, -1, 1, yRes);
    std::cout << "Clicked: " << xNDC << " " << -yNDC << std::endl;
    if (state == GLUT_DOWN) {
        currentSelection = findTriangle(xNDC, -yNDC);
        std::cout << "currentSelection: " << currentSelection << std::endl;
        glutPostRedisplay();
    } else {
        currentSelection = -1;
        currentDualFace = -1;
        glutPostRedisplay();
    }
    switch(button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN) {
                lastX = x;  // note where the drag began
                lastY = y;
                if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
                    userTransformType = 1;  // "1" indicates ZOOM! // TODO: MOVE TO MIDDLE MOUSE         
                } else {
                    userTransformType = 0; // "0" indicates TRANSLATION! // TODO: MOVE TO MIDDLE MOUSE
                }
            } else if (state == GLUT_UP) {
                userTransformType = -1;  // reset
            }
            break;
        case GLUT_MIDDLE_BUTTON:
            if (state == GLUT_DOWN) {
                lastX = x;
                lastY = y;
            } else if (state == GLUT_UP) {
                userTransformType = -1; // reset
            }
            break;
        case GLUT_RIGHT_BUTTON:
            if(state == GLUT_DOWN) {
                lastX = x;
                lastY = y; 
                userTransformType = 2;
            } else if (state == GLUT_UP) {
                userTransformType = -1; // reset
            }
            break;
    }
    downX = x;
    downY = y;
}    




/**
 * Main entrance point, obviously.
 * Sets up some stuff then passes control to glutMainLoop() which never
 * returns.
 */
int main(int argc, char* argv[])
{
    if (argc > 4 || argc < 3) {
        std::cerr << "USAGE: oglRenderer [xRes] [yRes] [Run]" << std::endl;
        exit(1);
    }

    xRes = atoi(argv[1]);
    yRes = atoi(argv[2]);

    assert(xRes >= 0);
    assert(yRes >= 0);

    readTex("smw2_yoshi_01_input.png");

    // OpenGL will take out any arguments intended for its use here.
    // Useful ones are -display and -gldebug.
    glutInit(&argc, argv);

    // Get a double-buffered, depth-buffer-enabled window, with an
    // alpha channel.
    // These options aren't really necessary but are here for examples.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(xRes, yRes);
    glutInitWindowPosition(300, 100);

    glutCreateWindow("CS176 Project");
    
    initGL();

    // set up GLUT callbacks.
    glutDisplayFunc(redraw);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyfunc);

    glutMouseFunc(mouseButton);

    glClearColor(0.0, 0.0, 0.0, 0.0); // set background color to black

    // From here on, GLUT has control,
    glutMainLoop();

    // so we should never get to this point.
    return 1;
}

