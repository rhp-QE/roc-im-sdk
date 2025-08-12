#pragma once

#include "imsdk/src/include/IMSDK.h"

namespace roc::imsdk::core {

class GroupManager : public roc::base::uncopyable {
public:
    GroupManager(std::weak_ptr<SDKRoot> w_sdk_root);
    ~GroupManager();

    // 组件加载完成后的初始化
    void all_component_did_load();

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
};

} // namespace roc::imsdk::core