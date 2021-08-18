#!/usr/bin/env python3

# Copyright 2020 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: ISC

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
