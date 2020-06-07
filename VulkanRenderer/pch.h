#pragma once

#define GLFW_INCLUDE_VULKAN
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <algorithm>
#include <any>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <math.h>
#include <optional>
#include <thread>
#include <typeindex>
#include <typeinfo>
#include <stdexcept>
#include <sstream>
#include <string>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

#include "boost/pfr/precise.hpp"
#include "fmt/format.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/hash.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "spdlog/spdlog.h"
#include "vulkan/vulkan.hpp"

namespace windows {
    #include <Windows.h>
}

namespace glfw {
    using namespace windows;
    #include "GLFW/glfw3.h"
}

namespace stb {
    #include "stb_image.h"
}

#include "tiny_obj_loader.h"