#pragma once

#include "imsdk/src/include/IMSDK.h"

namespace roc::imsdk::core {

class GroupManager : public roc::base::uncopyable {
public:
    GroupManager(std::shared_ptr<SDKRoot> sdk_root);
    ~GroupManager();

    // 组件加载完成后的初始化
    void AllComponentDidLoad();

private:
    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core