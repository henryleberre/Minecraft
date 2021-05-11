#pragma once

#include "header.hpp"

namespace mc {

	struct vec2f32 { union { struct { f32 x, y;       }; struct { f32 r, g;       }; }; };
	struct vec3f32 { union { struct { f32 x, y, z;    }; struct { f32 r, g, b;    }; }; };
	struct vec4f32 { union { struct { f32 x, y, z, w; }; struct { f32 r, g, b, a; }; }; };

}; // namespace mc