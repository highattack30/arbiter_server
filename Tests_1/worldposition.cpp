#include "worldposition.h"

#include <memory.h>
#include <math.h>
#include <float.h>

world_position * WINAPI wposition_clone(_In_ world_position * source)
{
	world_position * clone = new world_position;
	wposition_copy(source, clone);
	return clone;
}

bool WINAPI wposition_copy(_In_ world_position * src, _In_ world_position* dest)
{
	return memcpy(dest, src, sizeof(world_position)) ? true : false;
}

bool WINAPI wposition_has_collided(_In_ world_position *, _In_ world_position *)
{
	return false; //todo
}


float WINAPI wposition_distance(_In_ world_position * from, _In_ float x, _In_ float y, _In_ float z)
{
	if (!from)
		return FLT_MAX;

	float a = x - from->x.load(std::memory_order_relaxed);
	float b = y - from->y.load(std::memory_order_relaxed);
	float c = z - from->z.load(std::memory_order_relaxed);

	return sqrt(a* a + b*b + c*c);
}

float WINAPI wposition_distance(_In_ world_position * from, _In_ world_position *to)
{
	if (!from || !to)
		return FLT_MAX;

	float a = to->x.load(std::memory_order_relaxed) - from->x.load(std::memory_order_relaxed);
	float b = to->y.load(std::memory_order_relaxed) - from->y.load(std::memory_order_relaxed);
	float c = to->z.load(std::memory_order_relaxed) - from->z.load(std::memory_order_relaxed);

	return sqrt(a* a + b*b + c*c);
}

float WINAPI wposition_fast_distance(_In_ world_position* from, _In_ float x, _In_ float y)
{
	if (!from)
		return FLT_MAX;
	float a = x - from->x.load(std::memory_order_relaxed);
	float b = y - from->y.load(std::memory_order_relaxed);

	return sqrt(a* a + b*b);
}

float WINAPI wposition_fast_distance(_In_ world_position * from, _In_ world_position * to)
{
	if (!from || !to)
		return FLT_MAX;
	float a = to->x.load(std::memory_order_relaxed) - from->x.load(std::memory_order_relaxed);
	float b = to->y.load(std::memory_order_relaxed) - from->y.load(std::memory_order_relaxed);

	return sqrt(a* a + b*b) / 25.5f;
}

int16 WINAPI wposition_get_heading(_In_ world_position * from, _In_ world_position * to)
{
	return 0;
	//todo
}
