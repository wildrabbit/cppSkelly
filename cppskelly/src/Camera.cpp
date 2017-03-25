#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

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

