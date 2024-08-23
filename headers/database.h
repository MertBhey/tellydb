#include "config.h"
#include "resp.h"
#include "utils.h"

#include <stdint.h>

#include <pthread.h>

#ifndef DATABASE_H
  #define DATABASE_H

  #define TELLY_NULL 1
  #define TELLY_INT 2
  #define TELLY_STR 3
  #define TELLY_BOOL 4

  struct Transaction {
    struct Client *client;
    respdata_t *command;
  };

  struct KVPair {
    string_t key;

    union {
      string_t string;
      int integer;
      bool boolean;
      void *null;
    } value;

    uint32_t type;
  };

  void create_cache();
  void free_cache();

  struct KVPair *get_data(char *key, struct Configuration *conf);
  void set_data(struct KVPair pair);
  void save_data();

  void open_database_file(const char *filename);
  void close_database_file();

  pthread_t create_transaction_thread(struct Configuration *config);

  uint32_t get_transaction_count();
  void add_transaction(struct Client *client, respdata_t data);
  void remove_transaction(struct Transaction *transaction);
  void free_transactions();
#endif
