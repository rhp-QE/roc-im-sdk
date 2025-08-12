#include "imsdk/src/core/group/GroupManager.h"

namespace roc::imsdk::core {

GroupManager::GroupManager(std::weak_ptr<SDKRoot> w_sdk_root) : w_sdk_root_(w_sdk_root) {}

GroupManager::~GroupManager() = default;

void GroupManager::all_component_did_load() {
    // 组件加载完成后的初始化逻辑
}

} // namespace roc::imsdk::core