#!/usr/bin/env python2.7
import os
import sys
import argparse

import cauv.node
import cauv.messaging as messaging

parser = argparse.ArgumentParser(description="Send control messages to watch.py")

parser.add_argument("action", choices = ['start', 'stop', 'restart'], help='action to take on process')
parser.add_argument("process", help="process name to control")
parser.add_argument("--cmd", "-c", help="Command line to execute", nargs = argparse.REMAINDER, default = [])
parser.add_argument("--loud", help="Don't silence stdout", action = 'store_true')

args = parser.parse_args()

if not args.loud:
    #hacky hack hack
    null_fd = os.open("/dev/null", os.O_RDWR)
    os.dup2(null_fd, sys.stdout.fileno())

node = cauv.node.Node('watchctl.py')

action_map = {
    'start' : messaging.ProcessCommand.Start,
    'stop'  : messaging.ProcessCommand.Stop,
    'restart' : messaging.ProcessCommand.Restart,
}
node.send(messaging.ProcessControlMessage(action_map[args.action], args.process, args.cmd))
node.stop()