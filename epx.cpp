#include <iostream>
#include <vector>
#include <cmath>
#include "data.h"
#include "matrix.h"
#include "readpng.h"
#include "neighbors.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGl/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

void epx(int gHeight, int gWidth, int h, int w, int xRes, int yRes, int max, unsigned char gData[])
{
    GLubyte color[2*gHeight][2*gWidth][4];

    glClearColor(0.0, 0.0, 0.0, 0.0);
    //glVertex3f(0.95,-0.95,0);

    //GLubyte color[gHeight][gWidth][4];
    for(int y = 0; y < gHeight; y++) {
        for (int x = 0; x < gWidth; x ++) {
            int index = 4 * x + (4 * w * y);
            
            for (int xPixel = 0; xPixel < 2; xPixel++) {
                for (int yPixel = 0; yPixel < 2; yPixel++) {
                    color[2*y + yPixel][2*x + xPixel][0] = int(gData[index]);
                    color[2*y + yPixel][2*x + xPixel][1] = int(gData[index + 1]);
                    color[2*y + yPixel][2*x + xPixel][2] = int(gData[index + 2]);
                    color[2*y + yPixel][2*x + xPixel][3] = int(gData[index + 3]);
                }
            }

            int upIndex = getUpNeighbor(x,y, gWidth, gHeight);
            int downIndex = getDownNeighbor(x,y, gWidth, gHeight);
            int leftIndex = getLeftNeighbor(x,y, gWidth, gHeight);
            int rightIndex = getRightNeighbor(x,y, gWidth, gHeight);

            // this is for the corner cases
            if ((upIndex == -1 && leftIndex == -1) ||
                (upIndex == -1 && rightIndex == -1) ||
                (downIndex == -1 && leftIndex == -1) ||
                (downIndex == -1 && rightIndex == -1))
            {
                for (int xPixel = 0; xPixel < 2; xPixel++) {
                    for (int yPixel = 0; yPixel < 2; yPixel++) {
                        color[2*y + yPixel][2*x + xPixel][0] = int(gData[index]);
                        color[2*y + yPixel][2*x + xPixel][1] = int(gData[index + 1]);
                        color[2*y + yPixel][2*x + xPixel][2] = int(gData[index + 2]);
                        color[2*y + yPixel][2*x + xPixel][3] = int(gData[index + 3]);
                    }
                }
            }


            else
            {
                Vec3 up, down, left, right;

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

                bool upleftright = (up == left) && (up == right);
                bool upleftdown = (up == left) && (up == down);
                bool uprightdown = (up == right) && (up == down);
                bool leftrightdown = (left == down) && (right == down);
                bool all = (up == left) && (up == right) && (up == down) \
                            && (down == left) && (down == right) && (left == right);

                if (upLeft) {
                    color[2*y + 1][2*x][0] = int(up.x);
                    color[2*y + 1][2*x][1] = int(up.y);
                    color[2*y + 1][2*x][2] = int(up.z);
                    color[2*y + 1][2*x][3] = int(gData[upIndex+3]);
                }

                if (upRight) {
                    color[2*y + 1][2*x + 1][0] = int(up.x);
                    color[2*y + 1][2*x + 1][1] = int(up.y);
                    color[2*y + 1][2*x + 1][2] = int(up.z);
                    color[2*y + 1][2*x + 1][3] = int(gData[upIndex+3]);

                }

                if (downLeft) {
                    color[2*y][2*x][0] = int(down.x);
                    color[2*y][2*x][1] = int(down.y);
                    color[2*y][2*x][2] = int(down.z);
                    color[2*y][2*x][3] = int(gData[downIndex+3]);
                } 

                if (downRight) {
                    color[2*y][2*x + 1][0] = int(down.x);
                    color[2*y][2*x + 1][1] = int(down.y);
                    color[2*y][2*x + 1][2] = int(down.z);
                    color[2*y][2*x + 1][3] = int(gData[downIndex+3]);
                } 


                if (upleftright || upleftdown || uprightdown || leftrightdown || all) {
                    
                    color[2*y][2*x][0] = int(gData[index]);
                    color[2*y][2*x][1] = int(gData[index + 1]);
                    color[2*y][2*x][2] = int(gData[index + 2]);
                    color[2*y][2*x][3] = int(gData[index + 3]);

                    color[2*y][2*x + 1][0] = int(gData[index]);
                    color[2*y][2*x + 1][1] = int(gData[index + 1]);
                    color[2*y][2*x + 1][2] = int(gData[index + 2]);
                    color[2*y][2*x + 1][3] = int(gData[index + 3]);

                    color[2*y + 1][2*x][0] = int(gData[index]);
                    color[2*y + 1][2*x][1] = int(gData[index + 1]);
                    color[2*y + 1][2*x][2] = int(gData[index + 2]);
                    color[2*y + 1][2*x][3] = int(gData[index + 3]);

                    color[2*y + 1][2*x + 1][0] = int(gData[index]);
                    color[2*y + 1][2*x + 1][1] = int(gData[index + 1]);
                    color[2*y + 1][2*x + 1][2] = int(gData[index + 2]);
                    color[2*y + 1][2*x + 1][3] = int(gData[index + 3]);
                    
                }

            }
        }
    }
    glPixelZoom(floor(xRes/max)/2, floor(yRes/max)/2);

    glRasterPos2d(-1.0, -1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels( 2*gWidth, 2*gHeight, GL_RGBA, GL_UNSIGNED_BYTE, color );
    glFlush();
}
