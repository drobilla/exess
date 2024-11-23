// Copyright 2011-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#ifndef EXESS_TEST_NUM_TEST_UTILS_H
#define EXESS_TEST_NUM_TEST_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#  include <process.h>
#else
#  include <unistd.h>
#endif

typedef struct {
  size_t   n_tests;
  int64_t  low;
  int64_t  high;
  uint32_t seed;
  bool     exhaustive;
  bool     error;
} ExessNumTestOptions;

static inline bool
is_digit(const int c)
{
  return c >= '0' && c <= '9';
}

static bool
print_num_test_usage(const char* const name)
{
  fprintf(stderr, "Usage: %s [OPTION]...\n", name);
  fprintf(stderr,
          "  -h            Optional high value for exhaustive tests\n"
          "  -l            Optional low value for exhaustive tests\n"
          "  -n NUM_TESTS  Number of random tests to run\n"
          "  -s SEED       Random seed\n"
          "  -x            Exhaustively test numbers\n");
  return true;
}

static ExessNumTestOptions
parse_num_test_options(const int          argc,
                       char* const* const argv,
                       const size_t       default_n_tests,
                       const int64_t      min_rep,
                       const int64_t      max_rep)
{
  ExessNumTestOptions opts = {default_n_tests,
                              min_rep,
                              max_rep,
                              (uint32_t)time(NULL) + (uint32_t)getpid(),
                              false,
                              false};

  int a = 1;
  for (; a < argc && argv[a][0] == '-'; ++a) {
    if (argv[a][1] == 'h') {
      if (++a == argc) {
        opts.error = print_num_test_usage(argv[0]);
        break;
      }

      opts.high = (int64_t)strtoll(argv[a], NULL, 10);
    } else if (argv[a][1] == 'l') {
      if (++a == argc) {
        opts.error = print_num_test_usage(argv[0]);
        break;
      }

      opts.low = (int64_t)strtoll(argv[a], NULL, 10);
    } else if (argv[a][1] == 'x') {
      opts.exhaustive = true;
    } else if (argv[a][1] == 's') {
      if (++a == argc) {
        opts.error = print_num_test_usage(argv[0]);
        break;
      }

      opts.seed = (uint32_t)strtol(argv[a], NULL, 10);
    } else if (argv[a][1] == 'n') {
      if (++a == argc) {
        opts.error = print_num_test_usage(argv[0]);
        break;
      }

      opts.n_tests = (uint32_t)strtol(argv[a], NULL, 10);
    } else {
      opts.error = print_num_test_usage(argv[0]);
      break;
    }
  }

  return opts;
}

static void
print_num_test_progress(const uint64_t i, const uint64_t n_tests)
{
  if (i % (n_tests / 20) == 1) {
    fprintf(stderr, "%f%%\n", (double)i / (double)n_tests * 100.0);
  }
}

#endif // EXESS_TEST_NUM_TEST_UTILS_H
