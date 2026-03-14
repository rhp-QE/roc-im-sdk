//
// FrontierMessageUtility.h
//
// FrontierMessage 工具类，提供 track_id 等字段的提取方法
//

#pragma once

#include <cstdint>
#include <optional>

namespace roc::imsdk::network {

struct FrontierMessage;

/// FrontierMessage 工具类
class FrontierMessageUtility {
public:
    /// 从 FrontierMessage 的 metadata 中提取 track_id，统一转换为 uint32_t
    static std::optional<uint32_t> ExtractTrackId(const FrontierMessage& msg);
};

} // namespace roc::imsdk::network
