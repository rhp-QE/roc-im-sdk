//
// FrontierMessageUtility.cc
//
// FrontierMessage 工具类实现
//

#include "FrontierMessageUtility.h"
#include "imsdk/src/include/model/network.h"

namespace roc::imsdk::network {

std::optional<std::string> FrontierMessageUtility::ExtractTrackId(const FrontierMessage& msg) {
    auto it = msg.metadata.find("track_id");
    if (it != msg.metadata.end() && !it->second.empty()) {
        return it->second;
    }
    return std::nullopt;
}

} // namespace roc::imsdk::network
