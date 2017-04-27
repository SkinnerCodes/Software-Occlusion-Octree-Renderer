//const int NULL = 0;
#pragma once
#include <SDKDDKVer.h>
#include <windows.h>
#include <vector>

template<typename type> //aliasing a generic
using arraylist = std::vector<type>;

#define DONT_USE_CUSTOM_INT_T //define when you don't want glm/sdl to mess up int_t definitions, a custom gaurd, I modified the sdl/glm with the guard

#include <memory>
#include <iostream>
#include "Eigen/Core"
#include <algorithm>
#include <string>
#include "Utilities/sse util.h"
#include "Utilities/avx util.h"
#include "Utilities/bareMatrix.h"
#include "SDL/SDL.h"
#include "GL/glew.h"
#include "SDL/SDL_image.h"
#include <limits>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/detail/type_mat.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace glm;
using namespace sseutil;
using namespace avxutil;
using namespace Eigen;
using namespace std;
// using namespace SoftRendering;

