


#include "../libwavedrom/wavedrom.hpp"

#include <string>

namespace wavedrom
{


Wavedrom::Wavedrom()
	: Group("")
{

}


char * Wavedrom::Export()
{
	json_value* root = json_object_new(0);
	json_value* signals = this->ToJson();
	json_object_push(root, "signal", signals);
	char * buf = (char*)malloc(json_measure(root));
	json_serialize(buf, root);
	return buf;
}

}
