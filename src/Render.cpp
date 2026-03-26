#include "Engine.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float ZOOM_LEVEL = 1.0f;

Render::Render() : Module()
{
	name = "render";
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render()
{
}

// Called before render is available
bool Render::Awake()
{
	LOG("Create SDL rendering context");
	bool ret = true;

	int scale = Engine::GetInstance().window->GetScale() / 6;
	SDL_Window* window = Engine::GetInstance().window->window;

	//L05 TODO 5 - Load the configuration of the Render module
	
	// SDL3: no flags; create default renderer and set vsync separately
	renderer = SDL_CreateRenderer(window, nullptr);

	if (renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		if (configParameters.child("vsync").attribute("value").as_bool())
		{
			if (!SDL_SetRenderVSync(renderer, 1))
			{
				LOG("Warning: could not enable vsync: %s", SDL_GetError());
			}
			else
			{
				LOG("Using vsync");
			}
		}
		SDL_SetRenderLogicalPresentation(renderer, 1280, 720, SDL_LOGICAL_PRESENTATION_LETTERBOX);
		camera.w = Engine::GetInstance().window->width * scale;
		camera.h = Engine::GetInstance().window->height * scale;
		camera.x = 0;
		camera.y = 0;
		
	}
	/*if (!TTF_Init())
	{
		LOG("TTF_Init failed: %s", SDL_GetError());
		ret = false;
	}
	else
	{
		font = TTF_OpenFont("Assets/Fonts/alagard.ttf", 24);

		if (font == nullptr)
		{
			LOG("Failed to load font! SDL_ttf Error: %s", SDL_GetError());
		}
	}*/


	return ret;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// back background
	if (!SDL_GetRenderViewport(renderer, &viewport))
	{
		LOG("SDL_GetRenderViewport failed: %s", SDL_GetError());
	}
	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	return true;
}

bool Render::PostUpdate()
{
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);
	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_SetRenderViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
	SDL_SetRenderViewport(renderer, &viewport);
}

// Blit to screen
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, SDL_FlipMode flip) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	// SDL3 uses float rects for rendering
	SDL_FRect rect;
	float world_x = (float)((int)(camera.x * speed) + x * scale);
	float world_y = (float)((int)(camera.y * speed) + y * scale);
	float center_x = (float)(camera.w / 2);
	float center_y = (float)(camera.h / 2);

	rect.x = center_x + (world_x - center_x) * ZOOM_LEVEL;
	rect.y = center_y + (world_y - center_y) * ZOOM_LEVEL;
	

	if (section != NULL)
	{
		rect.w = (float)(section->w * scale * ZOOM_LEVEL);
		rect.h = (float)(section->h * scale * ZOOM_LEVEL);
	}
	else
	{
		float tw = 0.0f, th = 0.0f;
		if (!SDL_GetTextureSize(texture, &tw, &th)) { return false; }
		rect.w = tw * scale * ZOOM_LEVEL;
		rect.h = th * scale * ZOOM_LEVEL;
	}

	const SDL_FRect* src = NULL;
	SDL_FRect srcRect;
	if (section != NULL)
	{
		srcRect = { (float)section->x, (float)section->y, (float)section->w, (float)section->h };
		src = &srcRect;
	}

	SDL_FPoint* p = NULL;
	SDL_FPoint pivot;
	if (pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot = { (float)pivotX * scale * ZOOM_LEVEL, (float)pivotY * scale * ZOOM_LEVEL };
		p = &pivot;
	}

	// SDL3: returns bool; map to int-style check
	int rc = SDL_RenderTextureRotated(renderer, texture, src, &rect, angle, p, flip) ? 0 : -1;
	if (rc != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderTextureRotated error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_FRect rec;
	if (use_camera)
	{
		float world_x = (float)((int)(camera.x + rect.x * scale));
		float world_y = (float)((int)(camera.y + rect.y * scale));
		float center_x = (float)(camera.w / 2);
		float center_y = (float)(camera.h / 2);

		rec.x = center_x + (world_x - center_x) * ZOOM_LEVEL;
		rec.y = center_y + (world_y - center_y) * ZOOM_LEVEL;
		rec.w = (float)(rect.w * scale * ZOOM_LEVEL);
		rec.h = (float)(rect.h * scale * ZOOM_LEVEL);
	}
	else
	{
		rec = { (float)rect.x * scale, (float)rect.y * scale, (float)rect.w * scale, (float)rect.h * scale };
	}

	int result = (filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderRect(renderer, &rec)) ? 0 : -1;

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect/SDL_RenderRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawText(const char* text, int x, int y, int w, int h, SDL_Color color) const
{
	//if (!font || !renderer || !text) {
	//	LOG("DrawText: invalid font/renderer/text");
	//	return false;
	//}

	//// Render the text to a surface
	//// SDL3_ttf: length can be 0 for null-terminated strings
	//SDL_Surface* surface = TTF_RenderText_Solid(font, text, 0, color);
	//if (!surface) {
	//	LOG("DrawText: TTF_RenderText_Solid failed: %s", SDL_GetError());
	//	return false;
	//}

	//// Create a texture from the surface
	//SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	//if (!texture) {
	//	LOG("DrawText: SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
	//	SDL_DestroySurface(surface);
	//	return false;
	//}

	//// Optional but often needed when using alpha/text
	//SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	//// If w/h are 0, use the text’s natural size
	//float fw = (w > 0) ? (float)w : (float)surface->w;
	//float fh = (h > 0) ? (float)h : (float)surface->h;

	//SDL_FRect dstrect = { (float)x, (float)y, fw, fh };

	//// Render the texture to the current render target
	//if (!SDL_RenderTexture(renderer, texture, nullptr, &dstrect)) {
	//	LOG("DrawText: SDL_RenderTexture failed: %s", SDL_GetError());
	//}

	//// Cleanup
	//SDL_DestroyTexture(texture);
	//SDL_DestroySurface(surface);

	return true;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	float X1, Y1, X2, Y2;

	if (use_camera)
	{
		float center_x = (float)(camera.w / 2);
		float center_y = (float)(camera.h / 2);

		float world_x1 = (float)(camera.x + x1 * scale);
		float world_y1 = (float)(camera.y + y1 * scale);
		X1 = center_x + (world_x1 - center_x) * ZOOM_LEVEL;
		Y1 = center_y + (world_y1 - center_y) * ZOOM_LEVEL;

		float world_x2 = (float)(camera.x + x2 * scale);
		float world_y2 = (float)(camera.y + y2 * scale);
		X2 = center_x + (world_x2 - center_x) * ZOOM_LEVEL;
		Y2 = center_y + (world_y2 - center_y) * ZOOM_LEVEL;
	}
	else
	{
		X1 = (float)(x1 * scale);
		Y1 = (float)(y1 * scale);
		X2 = (float)(x2 * scale);
		Y2 = (float)(y2 * scale);
	}

	int result = SDL_RenderLine(renderer, X1, Y1, X2, Y2) ? 0 : -1;

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderLine error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_FPoint points[360];

	float factor = (float)M_PI / 180.0f;

	float cx = (float)((use_camera ? camera.x : 0) + x * scale);
	float cy = (float)((use_camera ? camera.y : 0) + y * scale);
	float scale_final;

	if (use_camera)
	{
		float screen_center_x = (float)(camera.w / 2);
		float screen_center_y = (float)(camera.h / 2);
		float world_x = (float)(camera.x + x * scale);
		float world_y = (float)(camera.y + y * scale);

		cx = screen_center_x + (world_x - screen_center_x) * ZOOM_LEVEL;
		cy = screen_center_y + (world_y - screen_center_y) * ZOOM_LEVEL;
		scale_final = (float)radius * scale * ZOOM_LEVEL;
	}
	else
	{
		cx = (float)(x * scale);
		cy = (float)(y * scale);
		scale_final= (float)radius * scale;
	}

	for (int i = 0; i < 360; ++i)
	{
		points[i].x = cx + (float)(radius * cos(i * factor));
		points[i].y = cy + (float)(radius * sin(i * factor));
	}

	result = SDL_RenderPoints(renderer, points, 360) ? 0 : -1;

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderPoints error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}
bool Render::IsOnScreenWorldRect(float x, float y, float w, float h, int margin) const
{
	bool result = false;

	// --- 1. Compute camera rectangle in WORLD space ---

	// camera.x and camera.y are render offsets (negative world position),
	// so we negate them to get the camera position in world coordinates.
	// The margin expands the camera rectangle to activate objects earlier.
	float camLeft = -camera.x - margin;
	float camTop = -camera.y - margin;

	// Right and bottom edges of the camera rectangle
	float camRight = camLeft + camera.w + margin * 2;
	float camBottom = camTop + camera.h + margin * 2;

	// --- 2. Compute object rectangle in WORLD space ---

	float objLeft = x;
	float objTop = y;
	float objRight = x + w;
	float objBottom = y + h;

	// --- 3. Axis-Aligned Bounding Box (AABB) overlap test ---

	result = objRight >= camLeft &&
		objLeft <= camRight &&
		objBottom >= camTop &&
		objTop <= camBottom;

	return result;
}

