#pragma once

#include "imsdk/src/include/IMSDK.h"

namespace roc::imsdk::core {

class GroupManager : public roc::base::uncopyable {
public:
    GroupManager(std::weak_ptr<SDKRoot> w_sdk_root);
    ~GroupManager();

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
};

} // namespace roc::imsdk::core