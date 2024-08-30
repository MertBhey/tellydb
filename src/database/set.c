#include "../../headers/telly.h"

#include <stddef.h>
#include <stdlib.h>

void set_data(struct KVPair pair) {
  struct BTree *cache = get_cache();
  struct KVPair *data = find_kv_from_btree(cache, pair.key.value);

  if (data != NULL) {
    if (data->type == TELLY_STR && pair.type != TELLY_STR) {
      free(data->value.string.value);
    }

    switch (pair.type) {
      case TELLY_STR:
        set_string(&data->value.string, pair.value.string.value, pair.value.string.len);
        break;

      case TELLY_INT:
        data->value.integer = pair.value.integer;
        break;

      case TELLY_BOOL:
        data->value.boolean = pair.value.boolean;
        break;

      case TELLY_NULL:
        data->value.null = NULL;
        break;
    }

    data->type = pair.type;
  } else {
    void *value = get_kv_val(&pair, pair.type);
    insert_kv_to_btree(cache, pair.key.value, value, pair.type);
  }
}
