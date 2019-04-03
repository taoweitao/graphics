#ifndef TRACKBALL_H
#define TRACKBALL_H

#include <glm/gtc/matrix_transform.hpp>

void vCalcRotVec(float fNewX, float fNewY,
                 float fOldX, float fOldY,
                 float fDiameter,
                 float *fVecX, float *fVecY, float *fVecZ);
void vAxisRotMatrix(float fVecX, float fVecY, float fVecZ, glm::mat4& mNewMat);

#endif
