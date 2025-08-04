#include "MessageORM.h"

namespace roc::imsdk::db {

MessageORM::MessageORM() = default;

MessageORM::~MessageORM() = default;

WCDB_CPP_ORM_IMPLEMENTATION_BEGIN(MessageORM)
WCDB_CPP_SYNTHESIZE(server_msg_id)
WCDB_CPP_SYNTHESIZE(client_msg_id)
WCDB_CPP_SYNTHESIZE(server_index)
WCDB_CPP_SYNTHESIZE(client_index)
WCDB_CPP_SYNTHESIZE(conv_id)
WCDB_CPP_SYNTHESIZE(sender_id)
WCDB_CPP_SYNTHESIZE(content)
WCDB_CPP_SYNTHESIZE(send_time)
WCDB_CPP_SYNTHESIZE(read_time)
WCDB_CPP_SYNTHESIZE(core_info)
WCDB_CPP_SYNTHESIZE(ext)
WCDB_CPP_SYNTHESIZE(is_deleted)
WCDB_CPP_SYNTHESIZE(is_recalled)
WCDB_CPP_ORM_IMPLEMENTATION_END

} // namespace roc::imsdk::db