#include "UIHelper.h"
#include "ApolloMath.h"
#include <iostream>
#include <sstream>

using namespace std;

namespace Apollo {

Color4f UIHelper::GetColorIntensity(FLOAT value) {
    value = CLAMP(value, 0, 1);
    if (value == 0) return Color4f::BLACK();
    value = (1 - value) / 3;
    Color4f color = Color4f::FromHSV(value * 360, 1, 1);
    return color;
}

string UIHelper::DisplayTime(UINT32 nMs) {
    UINT32 nHours = 0;
    UINT32 nMinutes = 0;
    UINT32 nSeconds = 0;
        
    nHours = nMs / (60 * 60 * 1000);
    nMs %= (60 * 60 * 1000);

    nMinutes = nMs / (60 * 1000);
    nMs %= (60 * 1000);
        
    nSeconds = nMs / 1000;
    nMs %= 1000;

    stringstream oss;

    if (nHours > 0) {
        oss << nHours << "h";
    }

    if (nHours > 0 || nMinutes > 0) {
        if (nMinutes < 10) {
            oss << "0" << nMinutes << "m";
        } else {
            oss << nMinutes << "m";
        }
    }

    if (nHours > 0 || nMinutes > 0 || nSeconds > 0) {
        if (nSeconds < 10) {
            oss << "0" << nSeconds << "s";
        } else {
            oss << nSeconds << "s";
        }
    }

    if (nHours > 0 || nMinutes > 0 || nSeconds > 0 || nMs > 0) {
        if (nMs < 10) {
            oss << "0" << nMs << "ms";
        } else {
            oss << nMs << "ms";
        }
    } else { 
        oss << "0ms";
    }
    return oss.str();
}

}
