#include "imsdk/src/core/group/GroupManager.h"

namespace roc::imsdk::core {

GroupManager::GroupManager(std::weak_ptr<SDKRoot> w_sdk_root) : w_sdk_root_(w_sdk_root) {}

GroupManager::~GroupManager() = default;

} // namespace roc::imsdk::core