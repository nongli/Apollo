#include "EngineCommon.h"

namespace Apollo {

static INT64 s_LUIDCounter = 1;

OBJECT_ID GetLUID() {
	return s_LUIDCounter++;
}

}
