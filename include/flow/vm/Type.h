#pragma once

namespace FlowVM {

enum class Type {
	Void = 0,
	Boolean = 1,        // bool (int64)
	Number = 2,         // int64
	String = 3,         // std::string* (TODO: later BufferRef)
	IPAddress = 5,      // IPAddress*
	Cidr = 6,           // Cidr*
	RegExp = 7,         // RegExp*
	Handler = 8,        // bool (*native_handler)(FlowContext*);
	StringArray = 9,    // Value[]
};

}
