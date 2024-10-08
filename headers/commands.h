#include "resp.h"
#include "server.h"

#include <stdint.h>

#ifndef COMMANDS_H
  #define COMMANDS_H

  struct Subcommand {
    char *name;
    char *summary;
    char *since;
    char *complexity;
  };

  struct Command {
    char *name;
    char *summary;
    char *since;
    char *complexity;
    void (*run)(struct Client *client, respdata_t *data, struct Configuration *conf);
    struct Subcommand *subcommands;
    uint32_t subcommand_count;
  };

  void execute_command(struct Client *client, respdata_t *data, struct Configuration *conf);

  void load_commands();
  struct Command *get_commands();
  uint32_t get_command_count();
  void free_commands();

  extern struct Command cmd_client;
  extern struct Command cmd_command;
  extern struct Command cmd_decr;
  extern struct Command cmd_get;
  extern struct Command cmd_incr;
  extern struct Command cmd_info;
  extern struct Command cmd_memory;
  extern struct Command cmd_ping;
  extern struct Command cmd_set;
  extern struct Command cmd_time;
  extern struct Command cmd_type;
#endif
