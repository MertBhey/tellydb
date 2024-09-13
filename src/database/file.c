#include "../../headers/telly.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <unistd.h>

static FILE *file = NULL;

void open_database_file(const char *filename) {
  file = fopen(filename, "r+");

  if (file == NULL) {
    fclose(fopen(filename, "w"));
    file = fopen(filename, "r+");
  }
}

FILE *get_database_file() {
  return file;
}

void close_database_file() {
  fclose(file);
}

uint32_t generate_data_content(char **line, struct KVPair *pair) {
  uint32_t len;

  switch (pair->type) {
    case TELLY_STR:
      len = pair->key.len + pair->value.string.len + 3;
      *line = malloc(len + 1);

      memcpy(*line, pair->key.value, pair->key.len);
      (*line)[pair->key.len] = 0x1D;
      (*line)[pair->key.len + 1] = TELLY_STR;
      memcpy(*line + pair->key.len + 2, pair->value.string.value, pair->value.string.len);
      (*line)[pair->key.len + 2 + pair->value.string.len] = 0x1E;
      (*line)[pair->key.len + 3 + pair->value.string.len] = 0x00;
      break;

    case TELLY_INT: {
      const uint32_t bit_count = log2(pair->value.integer) + 1;
      const uint32_t byte_count = (bit_count / 8) + 1;

      len = pair->key.len + byte_count + 3;
      *line = malloc(len + 1);

      memcpy(*line, pair->key.value, pair->key.len);
      (*line)[pair->key.len] = 0x1D;
      (*line)[pair->key.len + 1] = TELLY_INT;

      for (uint32_t i = 1; i <= byte_count; ++i) {
        (*line)[pair->key.len + 1 + i] = (pair->value.integer >> (8 * (byte_count - i))) & 0xFF;
      }

      (*line)[pair->key.len + 2 + byte_count] = 0x1E;
      (*line)[pair->key.len + 3 + byte_count] = 0x00;
      break;
    }

    case TELLY_BOOL:
      len = pair->key.len + 4;
      *line = malloc(len + 1);

      memcpy(*line, pair->key.value, pair->key.len);
      (*line)[pair->key.len] = 0x1D;
      (*line)[pair->key.len + 1] = TELLY_BOOL;
      (*line)[pair->key.len + 2] = pair->value.boolean;
      (*line)[pair->key.len + 3] = 0x1E;
      (*line)[pair->key.len + 4] = 0x00;
      break;

    case TELLY_NULL:
      len = pair->key.len + 3;
      *line = malloc(len + 1);

      memcpy(*line, pair->key.value, pair->key.len);
      (*line)[pair->key.len] = 0x1D;
      (*line)[pair->key.len + 1] = TELLY_NULL;
      (*line)[pair->key.len + 2] = 0x1E;
      (*line)[pair->key.len + 3] = 0x00;
      break;

    default:
      len = 0;
  }

  return len;
}

void save_data() {
  struct BTree *cache = get_cache();

  struct KVPair **pairs = get_sorted_kvs_from_btree(cache);
  const uint32_t size = get_total_size_of_node(cache->root);
  sort_kvs_by_pos(pairs, size);

  fseek(file, 0, SEEK_END);
  uint32_t file_size = ftell(file);
  int32_t diff = 0;

  for (uint32_t i = 0; i < size; ++i) {
    struct KVPair *pair = pairs[i];

    char *line = NULL;
    const uint32_t line_len = generate_data_content(&line, pair);

    if (pair->pos != -1) {
      uint32_t end_pos = pair->pos + 1;

      fseek(file, pair->pos + diff, SEEK_SET);
      while (fgetc(file) != 0x1E) end_pos += 1;

      const uint32_t line_len_in_file = end_pos - pair->pos;

      if (line_len_in_file != line_len) {
        char *buf = malloc(file_size - end_pos);
        fread(buf, sizeof(char), file_size - end_pos, file);
        fseek(file, pair->pos + diff, SEEK_SET);
        fwrite(line, sizeof(char), line_len, file);
        fwrite(buf, sizeof(char), file_size - end_pos, file);

        free(buf);

        diff += line_len - line_len_in_file;
      } else {
        fseek(file, pair->pos + diff, SEEK_SET);
        fwrite(line, sizeof(char), line_len, file);
      }
    } else {
      fseek(file, 0, SEEK_END);
      fwrite(line, sizeof(char), line_len, file);
      file_size += line_len;
    }

    free(line);
  }

  const int fd = fileno(file);
  ftruncate(fd, file_size + diff);

  free(pairs);
}
