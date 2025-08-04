#include "ConversationORM.h"

namespace roc::imsdk::db {

ConversationORM::ConversationORM() = default;

ConversationORM::~ConversationORM() = default;

WCDB_CPP_ORM_IMPLEMENTATION_BEGIN(ConversationORM)
WCDB_CPP_SYNTHESIZE(conv_id)
WCDB_CPP_SYNTHESIZE(name)
WCDB_CPP_SYNTHESIZE(conv_type)
WCDB_CPP_SYNTHESIZE(last_message_clent_id)
WCDB_CPP_SYNTHESIZE(last_message_server_id)
WCDB_CPP_SYNTHESIZE(last_message_time)
WCDB_CPP_SYNTHESIZE(unread_count)
WCDB_CPP_SYNTHESIZE(draft)
WCDB_CPP_SYNTHESIZE(is_top)
WCDB_CPP_SYNTHESIZE(is_muted)
WCDB_CPP_SYNTHESIZE(is_deleted)
WCDB_CPP_SYNTHESIZE(delete_time)
WCDB_CPP_SYNTHESIZE(is_blocked)
WCDB_CPP_SYNTHESIZE(core_info)
WCDB_CPP_SYNTHESIZE(ext)
WCDB_CPP_SYNTHESIZE(mask)
WCDB_CPP_ORM_IMPLEMENTATION_END

} // namespace roc::imsdk::db