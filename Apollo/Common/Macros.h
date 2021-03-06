#pragma once

#ifdef SAFE_RELEASE
#undef SAFE_RELEASE
#endif
#define SAFE_RELEASE(x) if (x) x->Release(), x = nullptr

#ifdef SAFE_DELETE
#undef SAFE_DELETE
#endif
#define SAFE_DELETE(x) if (x) delete x, x = nullptr;

#ifdef SAFE_ARRAY_DELETE
#undef SAFE_ARRAY_DELETE
#endif
#define SAFE_ARRAY_DELETE(x) if (x) delete[] x, x = nullptr;

#ifdef UNREFERENCED_PARAMETER
#undef UNREFERENCED_PARAMETER
#endif
#define UNREFERENCED_PARAMETER(a) a
