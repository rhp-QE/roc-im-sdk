#include "WCDB/WCDBCpp.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"

namespace roc::imsdk::core::message::dbopt {

bool insert_message(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages);

} // namespace roc::imsdk::core::message::dbopt