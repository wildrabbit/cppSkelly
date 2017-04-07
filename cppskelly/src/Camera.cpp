#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

void initOrtho(OrthoCamera* cam, const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up, const glm::vec4& borders, float zNear, float zFar)
{
	cam->eye = eye;
	cam->target = target;
	cam->up = up;
	cam->left = borders[0];
	cam->right = borders[1];
	cam->top = borders[2];
	cam->bot = borders[3];
	cam->zNear = zNear;
	cam->zFar = zFar;
}

void initPerspective(PerspectiveCamera* cam, const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up, float fov, float aspect, float zNear, float zFar)
{
	cam->eye = eye;
	cam->target = target;
	cam->up = up;
	cam->fov = fov;
	cam->aspect = aspect;
	cam->zNear = zNear;
	cam->zFar = zFar;
}

void updateCameraViewMatrix(Camera* cam)
{
	cam->viewMatrix = glm::lookAt(cam->eye, cam->target, cam->up);
}

void updateCameraProjectionMatrix(PerspectiveCamera* cam)
{
	cam->projMatrix = glm::perspective(cam->fov, cam->aspect, cam->zNear, cam->zFar);
}

void updateCameraProjectionMatrix(OrthoCamera* cam)
{
	cam->projMatrix = glm::ortho(cam->left, cam->right, cam->bot, cam->top, cam->zNear, cam->zFar);
}

