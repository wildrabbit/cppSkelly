#ifndef CAMERAH_H
#define CAMERAH_H
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct Camera
{
	glm::vec3 eye;
	glm::vec3 up;
	glm::vec3 target;

	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	float zNear;
	float zFar;
};

struct PerspectiveCamera : Camera
{
	float fov;
	float aspect;
};

struct OrthoCamera : Camera
{
	float left;
	float right;
	float top;
	float bot;
};



//void rotateCamera(OrthoCamera* cam, const glm::vec3& axis, float angle)
//{
//	cam->viewMatrix = rotate(cam->viewMatrix, angle, axis);
//	cam->up = cam->up * glm::rotate(glm::mat4(1.0f), angle, axis);
//	cam->target = rotate(cam->target, angle, axis);
//}
//
//void translateCamera(OrthoCamera* cam, const glm::vec3& translationVector)
//{
//	cam->viewMatrix = translate(cam->viewMatrix, translationVector);
//	cam->eye = translate(cam->eye, translationVector);
//	cam->target = translate(cam->eye)
//}
//
//void zoomCamera(OrthoCamera* cam, float value)
//{
//	float newWidth = (cam->right - cam->left) / value;
//	cam->left = 
//}

void initOrtho(OrthoCamera* cam, const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up, const glm::vec4& borders, float zNear, float zFar);
void initPerspective(PerspectiveCamera* cam, const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up, float fov, float aspect, float zNear, float zFar);

void updateCameraViewMatrix(Camera* cam);
void updateCameraProjectionMatrix(PerspectiveCamera* cam);
void updateCameraProjectionMatrix(OrthoCamera* cam);


static OrthoCamera gCam;
static PerspectiveCamera gPerspectiveCam;


#endif