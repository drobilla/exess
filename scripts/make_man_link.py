#!/usr/bin/env python3

# Copyright 2020 David Robillard <d@drobilla.net>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

"""
Make a man page "link" that simply includes another page.
"""

import argparse
import sys


def run(target, link):
    """Create a link manpage that includes target."""

    with open(link, "w") as link_file:
        link_file.write(".so {}\n".format(target))


def run_command(args):
    """Run using command line arguments."""

    parser = argparse.ArgumentParser(
        usage="%(prog)s TARGET LINK",
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    parser.add_argument(
        "target",
        help="name of target man page to link to",
    )

    parser.add_argument(
        "link",
        help="name of link alias page to create",
    )

    run(**vars(parser.parse_args(args)))


if __name__ == "__main__":
    run_command(sys.argv[1:])
