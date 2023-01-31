#! /usr/bin/python3
# vim: set sw=2 ts=8 sts=2 et:
#
# Copyright (c)
#   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
# 
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA 02111-1307 USA.

#from __future__ import print_function

import sys
import argparse
from subprocess import Popen, PIPE
import os
from os import path
import re
import glob

def eprint(*args, **kwargs):
  print(*args, file=sys.stderr, **kwargs)

def main():
  PROG=path.basename(sys.argv[0])

  help='''This runs a Program Under Test (PUT) with the given options and stores the PUTs stdout into a log file.
The log file will be deleted if the PUT does not exit successfully.
In this case, {prog} will also die.'''.format(prog=PROG)

  parser = argparse.ArgumentParser(description=help, prog=PROG)
  parser.add_argument("-l", "--log", type=str, help="file used to store the stdout of the PUT", required=True)
  parser.add_argument("-c", "--clean", type=str, help="further file globs to clean if the PUT dies")
  parser.add_argument("--filter", type=str, help="only log stuff matching the filter regex")
  parser.add_argument('PUT', help='program under test')
  parser.add_argument('opt', nargs='*', help='options for the PUT')
  args = parser.parse_args()

# print(args)


  if args.filter is not None:
    try:
      filter = re.compile(args.filter)
    except re.error as e:
      eprint("Invalid regular expression '{}':".format(args.filter), e)
      return -1
  else:
    filter = re.compile("^.?")


  try:
    PUT = Popen([args.PUT]+args.opt, stdout=PIPE, stderr=PIPE)
  except (IOError, OSError) as e:
    eprint("Can't start '{}':".format(args.PUT), e)
    return -1

  try:
    LOG = open(args.log, 'w')
  except (IOError, OSError) as e:
    eprint("Can't open log file '{}':".format(args.log), e)
    return -1

  out = dict()

  for line_ in PUT.stdout:
    line = line_.decode()
    if line == "Info: /OSCI/SystemC: Simulation stopped by user.\n":
      continue 
    m = filter.match(line)
    if m:
      if filter.groups >= 1:
        if m.group(1) not in out:
          out[m.group(1)] = []
        out[m.group(1)].append(line)
      else:
        LOG.write(line)

  for key in sorted(out.keys()):
    for line in out[key]:
      LOG.write(line)

  PUT.wait()
  if PUT.returncode != 0:
    eprint("The PUT '{}' has died: {}".format(args.PUT, PUT.returncode))
    for line in PUT.stderr:
      eprint(line, end='')
    os.remove(args.log)
    if args.clean:
      for clean in glob.iglob(args.clean):
        os.remove(clean)
    return PUT.returncode

if __name__ == '__main__':
  exit(main())
