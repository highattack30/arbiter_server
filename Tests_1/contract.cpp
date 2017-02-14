#include "contract.h"
#include "connexion.h"
#include "contract_manager.h"
#include "chat.h"

contract::contract(e_contract_type t, uint32 i) :id(i), type(t), root(nullptr), owner(nullptr) {}
contract::~contract() {
	root = nullptr;
	data.Clear();
}

bool contract::init(p_ptr p, contract_manager * r, Stream & d)
{
	if (!p || !r) return false;
	owner = std::move(p);
	root = r;

	data.Resize(d._size);
	data.Write(d._raw, d._size);
	data.SetFront();

	Stream d_0;
	d_0.Resize(8);
	d_0.WriteUInt16(8);
	d_0.WriteUInt16(S_REPLY_REQUEST_CONTRACT);
	d_0.WriteInt32(type);
	if (!connection_send(owner->con, &d_0))
		return false;

	if (internal_init()) return true;

	root->end_contract(id);

	return false;
}

void contract::cancel()
{
	send_social_cancel(owner);
	data.Clear();
	data.Resize(28);
	data.WriteInt16(28);
	data.WriteInt16(S_CANCEL_CONTRACT);
	data.WriteWorldId(owner);
	data._pos += 8;
	data.WriteUInt32(type);
	data.WriteUInt32(id);
	connection_send(owner->con, &data);

	root->end_contract(id);
}
