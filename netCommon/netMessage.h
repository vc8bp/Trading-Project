#pragma once
#include "netCommon.h"


namespace olc {
	namespace net {
		template <typename T>
		struct message_header {
			T id{};

		};
	}
}
