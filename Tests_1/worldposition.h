#ifndef WORLDPOSITION_H
#define WORLDPOSITION_H

#include "typeDefs.h"
#include <sal.h>
#include <atomic>

typedef struct world_position
{
	std::atomic<float>		x;
	std::atomic<float>		y;
	std::atomic<float>		z;
	std::atomic<float>		t_x;
	std::atomic<float>		t_y;
	std::atomic<float>		t_z;
	std::atomic_int16_t		heading;

	std::atomic_int32_t
							channel,
							continent_id,
							worldMapSectionId,
							worldMapWorldId,
							worldMapGuardId;

} world_position;

world_position * WINAPI wposition_clone(_In_ world_position * src);
bool WINAPI wposition_copy(_In_ world_position *src, _In_ world_position* dest);
float WINAPI wposition_distance(_In_ world_position * from, _In_ world_position *to);
float WINAPI wposition_distance(_In_ world_position *from, _In_ float x, _In_ float y, _In_ float z);
float WINAPI wposition_fast_distance(_In_ world_position *from, _In_ float z, _In_ float y);
float WINAPI wposition_fast_distance(_In_ world_position *from, _In_ world_position *to);
int16 WINAPI wposition_get_heading(_In_ world_position *from, _In_ world_position *to);

#endif
