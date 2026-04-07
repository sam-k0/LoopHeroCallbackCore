#pragma once

enum OriginalCall {
	CANCELLED, // the call was cancelled
	CALLED, // the call was done
	EARLY // we are in prepatch, too early to know
};

struct CallbackCoreAttributes {
	int call;
	// may add more fields in the future

	CallbackCoreAttributes(int call)
	{
		this->call = call;
	};
};

