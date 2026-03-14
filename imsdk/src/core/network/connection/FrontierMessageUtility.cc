//
// FrontierMessageUtility.cc
//
// FrontierMessage 工具类实现
//

#include "FrontierMessageUtility.h"
#include "imsdk/src/include/model/network.h"
#include <string>

namespace roc::imsdk::network {

std::optional<uint32_t> FrontierMessageUtility::ExtractTrackId(const FrontierMessage& msg) {
    auto it = msg.metadata.find("track_id");
    if (it == msg.metadata.end() || it->second.empty()) {
        return std::nullopt;
    }
    try {
        return static_cast<uint32_t>(std::stoul(it->second));
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace roc::imsdk::network
