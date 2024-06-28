// https://cplusplus.com/forum/beginner/51572/
#pragma once
#include <string>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {

	std::string base64_encode(unsigned char const*, unsigned int len);
	std::string base64_decode(std::string const& s);

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
