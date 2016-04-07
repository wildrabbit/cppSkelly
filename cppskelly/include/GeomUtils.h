#ifndef GEOMUTILS_H
#define GEOMUTILS_H

const float PI = std::acosf(-1.0f);

const float DEG2RAD = PI / 180.0f;
const float RAD2DEG = 1.0f / DEG2RAD;

enum class PivotType
{
	TopLeft,
	Top,
	TopRight,
	CentreLeft,
	Centre,
	CentreRight,
	BotLeft,
	Bottom,
	BotRight,
	Custom
};

struct Rect
{
	float x, y, w, h;

	Rect() : x(0.0f), y(0.0f), w(0.0f), h(0.0f){}

	inline bool empty()
	{
		return w < FLT_EPSILON || h < FLT_EPSILON;
	}
};

#endif