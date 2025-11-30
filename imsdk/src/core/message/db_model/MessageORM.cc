#include "MessageORM.h"

namespace roc::imsdk::core::message {

MessageORM::MessageORM() = default;

MessageORM::~MessageORM() = default;

WCDB_CPP_ORM_IMPLEMENTATION_BEGIN(MessageORM)

// 字段绑定
WCDB_CPP_SYNTHESIZE(status)
WCDB_CPP_SYNTHESIZE(is_pinned)
WCDB_CPP_SYNTHESIZE(is_deleted)
WCDB_CPP_SYNTHESIZE(is_recalled)
WCDB_CPP_SYNTHESIZE(is_group_msg)
WCDB_CPP_SYNTHESIZE(content)
WCDB_CPP_SYNTHESIZE(to_user_id)
WCDB_CPP_SYNTHESIZE(from_user_id)
WCDB_CPP_SYNTHESIZE(client_msg_id)
WCDB_CPP_SYNTHESIZE(server_msg_id)
WCDB_CPP_SYNTHESIZE(conversation_id)
WCDB_CPP_SYNTHESIZE(client_order_index)
WCDB_CPP_SYNTHESIZE(server_order_index)
WCDB_CPP_SYNTHESIZE(send_time)
WCDB_CPP_SYNTHESIZE(sync_ext)
WCDB_CPP_SYNTHESIZE(local_ext)
WCDB_CPP_SYNTHESIZE(propertys)

// 字段约束
WCDB_CPP_PRIMARY(client_msg_id)

// 索引
WCDB_CPP_INDEX_DESC("_index", client_order_index)

WCDB_CPP_ORM_IMPLEMENTATION_END

} // namespace roc::imsdk::core::message