#include "imsdk/src/core/group/GroupManager.h"

namespace roc::imsdk::core {

GroupManager::GroupManager(std::shared_ptr<SDKRoot> sdk_root) : w_sdk_root(sdk_root) {}

GroupManager::~GroupManager() = default;

void GroupManager::all_component_did_load() {
    // 组件加载完成后的初始化逻辑
}

} // namespace roc::imsdk::core