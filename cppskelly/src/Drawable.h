#ifndef DrawableH_H
#define DrawableH_H

struct SDL_Window;
struct Camera;
struct Drawable
{
	virtual void draw(SDL_Window* w, Camera* c) = 0;
	virtual void cleanup() = 0;
};
#endif
