#pragma once

#include "core/sdkroot/SDKRoot.h"
#include "imsdk/base/include/uncopyable.h"
#include <memory>

namespace roc::imsdk::core::conversation {

class ConversationStatusHandler : public base::uncopyable {

public:

    ConversationStatusHandler(std::weak_ptr<SDKRoot> root);

    void AllComponentDidLoad();

private:
    std::weak_ptr<SDKRoot> w_sdk_root;
};

}