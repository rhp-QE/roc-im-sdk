#include "ConversationORM.h"

namespace roc::imsdk::core::conversation {

ConversationORM::ConversationORM() = default;

ConversationORM::~ConversationORM() = default;

WCDB_CPP_ORM_IMPLEMENTATION_BEGIN(ConversationORM)
WCDB_CPP_SYNTHESIZE(type)
WCDB_CPP_SYNTHESIZE(name)
WCDB_CPP_SYNTHESIZE(unread_count)
WCDB_CPP_SYNTHESIZE(avatar_url)
WCDB_CPP_SYNTHESIZE(last_message_time)
WCDB_CPP_SYNTHESIZE(conversation_id)
WCDB_CPP_SYNTHESIZE(last_message_client_id)
WCDB_CPP_SYNTHESIZE(last_message_server_id)
WCDB_CPP_SYNTHESIZE(is_top)
WCDB_CPP_SYNTHESIZE(mask)
WCDB_CPP_SYNTHESIZE(is_muted)
WCDB_CPP_SYNTHESIZE(is_deleted)
WCDB_CPP_SYNTHESIZE(is_blocked)
WCDB_CPP_SYNTHESIZE(draft)
WCDB_CPP_SYNTHESIZE(sync_ext)
WCDB_CPP_SYNTHESIZE(local_ext)
WCDB_CPP_ORM_IMPLEMENTATION_END

} // namespace roc::imsdk::core::conversation