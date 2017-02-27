#ifndef INTERNALOP_H
#define INTERNALOP_H

#include "typeDefs.h"
#include "job.h"
#include <memory>

class connection;

enum e_io_opcodes :uint16 {
	IO_REMOVE_PLAYER = 1,
	IO_LOAD_TOPO_FIN,
	IO_CHAT,

	IO_MAX
};

typedef void (WINAPI * io_fnunction)(std::shared_ptr<connection>, Stream&);

static io_fnunction io_fn[IO_MAX];

void WINAPI io_init();
void WINAPI io_exectue(e_io_opcodes, std::shared_ptr<connection>, Stream&);


void WINAPI io_player_enter_world(std::shared_ptr<connection>, uint8 result, std::vector<uint32>&);

void WINAPI io_load_topo_fin(std::shared_ptr<connection> c, Stream&);
void WINAPI io_remove_player(std::shared_ptr<connection>, Stream&);
void WINAPI io_chat(std::shared_ptr<connection>, Stream&);
#endif // 
