/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@freedesktop.org>
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "rendertest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct render_int_arg_state {
  render_arg_state_t *state;
  char *program_name;
  const render_option_t *backend_options;
  render_parser_t backend_parser;
} render_int_arg_state_t;

static const render_option_t _options[] = {
  { "clip",        'c', "CLIP", 0, "         use this clip primitive" },
  { "interactive", 'i', NULL,   0, "      prompt before all tests" },
  { "npot",        'n', NULL,   0,
    "             use non power of two sized surfaces" },
  { "operator",    'o', "OP",   0, "       use this porter/duff operator" },
  { "quiet",       'q', NULL,   0,
    "            do not explain what is being done" },
  { "repeat",      'r', "ITERATIONS", 0,
    " repeat every test this many times" },
  { "sleep",       's', "SEC",  0,
    "         sleep this number of seconds between tests" },
  { "tests",       't', "TESTSPAN", 0, "    only run these tests" },
  { "time",        'm',  NULL,  0, "             print timing statistics" },
  { "format",      'f', "FORMAT", 0, "     use this source surface format" },
  { 0 }
};

static const render_option_t _additional_options[] = {
  { "help",    'h', NULL, 1, "             display this help and exit" },
  { "version", 'v', NULL, 1, "          output version information and exit" },
  { 0 }
};

static void
_print_option (const render_option_t *option)
{

  printf (" %c%c%c --%s%c%s %s\n",
	  (option->flags)? ' ': '-',
	  (option->flags)? ' ': option->key,
	  (option->flags)? ' ': ',',
	  option->name,
	  (option->arg)? '=': ' ',
	  (option->arg)? option->arg: "",
	  option->doc);
}

static void
_rendertest_usage (render_int_arg_state_t *state)
{
  int i;

  printf ("Usage: %s [OPTION]...\n"
	  "Test utility for RENDER-like graphics systems.\n"
	  "\n"
	  "Options:\n", state->program_name);

  for (i = 0; _options[i].name; i++)
    _print_option (&_options[i]);

  if (state->backend_options->name) {
    printf ("\nBackend specific options:\n");

    for (i = 0; state->backend_options[i].name; i++)
      _print_option (&state->backend_options[i]);

    printf ("\n");
  }

  for (i = 0; _additional_options[i].name; i++)
    _print_option (&_additional_options[i]);

  printf ("\nReport bugs to <davidr@freedesktop.org>.\n");
}

static render_operator_t
_render_operator (char *op)
{
  if (strcasecmp (op, "CLEAR") == 0)
    return RENDER_OPERATOR_CLEAR;
  else if (strcasecmp (op, "SRC") == 0)
    return RENDER_OPERATOR_SRC;
  else if (strcasecmp (op, "DST") == 0)
    return RENDER_OPERATOR_DST;
  else if (strcasecmp (op, "OVER_REVERSE") == 0)
    return RENDER_OPERATOR_OVER_REVERSE;
  else if (strcasecmp (op, "IN") == 0)
    return RENDER_OPERATOR_IN;
  else if (strcasecmp (op, "IN_REVERSE") == 0)
    return RENDER_OPERATOR_IN_REVERSE;
  else if (strcasecmp (op, "OUT") == 0)
    return RENDER_OPERATOR_OUT;
  else if (strcasecmp (op, "OUT_REVERSE") == 0)
    return RENDER_OPERATOR_OUT_REVERSE;
  else if (strcasecmp (op, "ATOP") == 0)
    return RENDER_OPERATOR_ATOP;
  else if (strcasecmp (op, "ATOP_REVERSE") == 0)
    return RENDER_OPERATOR_ATOP_REVERSE;
  else if (strcasecmp (op, "XOR") == 0)
    return RENDER_OPERATOR_XOR;
  else if (strcasecmp (op, "ADD") == 0)
    return RENDER_OPERATOR_ADD;
  else
    return RENDER_OPERATOR_OVER;
}

static render_clip_t
_render_clip (char *clip)
{
  if (strcasecmp (clip, "RECTS") == 0)
    return RENDER_CLIP_RECTANGLES;
  else if (strcasecmp (clip, "TRAPS") == 0)
    return RENDER_CLIP_TRAPEZOIDS;
  else
    return RENDER_CLIP_NONE;
}

static render_format_t
_render_format (char *format)
{
  if (strcasecmp (format, "RGB24") == 0)
    return RENDER_FORMAT_RGB24;
  if (strcasecmp (format, "ARGB32") == 0)
    return RENDER_FORMAT_ARGB32;
  else if (strcasecmp (format, "YV12") == 0)
    return RENDER_FORMAT_YV12;
  else if (strcasecmp (format, "YUY2") == 0)
    return RENDER_FORMAT_YUY2;
  else
    return RENDER_FORMAT_ARGB32;
}

static int
_parse_option (int key, char *arg, render_int_arg_state_t *s)
{
  switch (key) {
  case 'c':
    s->state->settings.clip = _render_clip (arg);
    break;
  case 'i':
    s->state->settings.interactive = 1;
    break;
  case 'm':
    s->state->settings.time = 1;
    break;
  case 'n':
    s->state->settings.npot = 1;
    break;
  case 'o':
    s->state->settings.op = _render_operator (arg);
    break;
  case 'q':
    s->state->settings.quiet = 1;
    break;
  case 'r':
    s->state->settings.repeat = atoi (arg);
    if (s->state->settings.repeat < 1)
      s->state->settings.repeat = 1;
    break;
  case 's':
    s->state->settings.sleep = atoi (arg);
    break;
  case 't': {
    long int t;
    char *next = NULL;

    t = strtol (arg, &next, 0);
    if (t < 0)
      s->state->settings.last_test = -t;
    else {
      s->state->settings.first_test = t;

      if (next && *next != '\0') {
	t = strtol (next, NULL, 0);
	if (t)
	  s->state->settings.last_test = -t;
      } else
	s->state->settings.last_test = s->state->settings.first_test;
    }
  } break;
  case 'v':
    printf ("rendertest - " VERSION "\n");
    return 100;
  case 'h':
    _rendertest_usage (s);
    return 200;
 case 'f':
      s->state->settings.format = _render_format (arg);
    break;
  default:
    return s->backend_parser (key, arg, s->state);
  }

  return 0;
}

static const render_option_t *
_find_matching_short_option (int key,
			     const render_option_t *backend_options)
{
  int i;

  for (i = 0; _options[i].name; i++)
    if (key == _options[i].key)
      return &_options[i];

  for (i = 0; backend_options[i].name; i++)
    if (key == backend_options[i].key)
      return &backend_options[i];

  for (i = 0; _additional_options[i].name; i++)
    if (key == _additional_options[i].key)
      return &_additional_options[i];

  return NULL;
}

static const render_option_t *
_find_matching_long_option (char *name,
			    const render_option_t *backend_options)
{
  int i;

  for (i = 0; _options[i].name; i++)
    if (strcmp (name, _options[i].name) == 0)
      return &_options[i];

  for (i = 0; backend_options[i].name; i++)
    if (strcmp (name, backend_options[i].name) == 0)
      return &backend_options[i];

  for (i = 0; _additional_options[i].name; i++)
    if (strcmp (name, _additional_options[i].name) == 0)
      return &_additional_options[i];

  return NULL;
}

int
render_parse_arguments (render_parser_t parser,
			const render_option_t *backend_options,
			render_arg_state_t *state,
			int argc, char **argv)
{
  int i, j, status, skip;
  const render_option_t *option;
  char *arg;
  render_int_arg_state_t s;

  s.state = state;
  s.program_name = argv[0];
  s.backend_parser = parser;
  s.backend_options = backend_options;

  state->settings.interactive = 0;
  state->settings.npot = 0;
  state->settings.quiet = 0;
  state->settings.sleep = 5;
  state->settings.repeat = 1;
  state->settings.op = RENDER_OPERATOR_OVER;
  state->settings.first_test = 0;
  state->settings.last_test = 65535;
  state->settings.time = 0;
  state->settings.clip = RENDER_CLIP_NONE;
  state->settings.format = RENDER_FORMAT_ARGB32;

  for (i = 1; i < argc; i++) {
    if (argv[i][0] != '-')
      continue;

    status = skip = 0;

    if (argv[i][1] == '-') {
      char *eq;

      eq = strchr (argv[i], '=');
      if (eq) {
	arg = eq + 1;
	*eq = '\0';
      } else
	arg = NULL;

      option = _find_matching_long_option (&argv[i][2], backend_options);
      if (option) {
	if (option->arg) {
	  if (!arg) {
	    if (argc > (i + 1)) {
	      arg = argv[i + 1];
	      skip = 1;
	    } else {
	      printf ("%s: option '--%s' requires an argument\n",
		      s.program_name, option->name);
	      printf ("Try '%s --help' for more information.\n",
		      s.program_name);

	      return 1;
	    }
	  }
	} else if (arg) {
	  printf ("%s: option '--%s' doesn't allow an argument\n",
		  s.program_name, option->name);
	  printf ("Try '%s --help' for more information.\n", s.program_name);

	  return 1;
	}

	status = _parse_option (option->key, arg, &s);
	if (status) {
	  if (status != 100 && status != 200)
	    printf ("Try '%s --help' for more information.\n", s.program_name);

	  return 1;
	}
      } else {
	printf ("%s: unrecognized option '--%s'\n",
		s.program_name, &argv[i][2]);
	printf ("Try '%s --help' for more information.\n", s.program_name);

	return 1;
      }
    } else {
      for (j = 1; argv[i][j] != '\0'; j++) {
	option = _find_matching_short_option (argv[i][j], backend_options);
	if (option) {
	  if (option->arg) {
	    if (argv[i][j + 1] != '\0') {
	      arg = &argv[i][j + 1];
	      j += strlen (&argv[i][j + 1]);
	    } else if (argc > (i + 1)) {
	      arg = argv[i + 1];
	      skip = 1;
	    } else {
	      printf ("%s: option requires an argument -- %c\n",
		      s.program_name, option->key);
	      printf ("Try '%s --help' for more information.\n",
		      s.program_name);

	      return 1;
	    }
	  } else
	    arg = NULL;

	  status = _parse_option (option->key, arg, &s);
	  if (status) {
	    printf ("Try '%s --help' for more information.\n", s.program_name);

	    return 1;
	  }
	} else {
	  printf ("%s: invalid option -- %c\n", s.program_name, argv[i][j]);
	  printf ("Try '%s --help' for more information.\n", s.program_name);

	  return 1;
	}
      }
    }

    i += skip;
  }

  return 0;
}
