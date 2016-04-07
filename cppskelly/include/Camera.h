#ifndef CAMERAH_H
#define CAMERAH_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "GeomUtils.h"
#include "glad/glad.h"

class Camera
{
protected:
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
public:
	Camera();
	virtual ~Camera();

	virtual void init() = 0;
};

class OrthoCamera: public Camera
{
private:
	glm::vec3 eye;
	glm::vec3 up;
	glm::vec3 target;

	float left;
	float right;
	float top;
	float bot;
	float zNear;
	float zFar;
public:
	OrthoCamera();
	virtual ~OrthoCamera();

	virtual void init() override;
};

// TODO: Perspective camera.
#endif