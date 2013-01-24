#!/usr/bin/env python2.7
#
# Copyright 2013 Cambridge Hydronautics Ltd.
#
# See license.txt for details.
#

import cauv.node
from cauv import control
from cauv.debug import debug, warning, error, info

import time
import threading, Queue
import argparse
import utils.event as event
import utils.morse

from AI.base.proc import aiProcess, external_function, force_calling_process

#TODO basically the actual functionality of control, the ability to stop the sub, block script_ids etc

class slightlyModifiedAUV(control.AUV):
    def __init__(self, node):
        control.AUV.__init__(self, node)
        self.depth_limit = None
        self.prop_limit = 127
    def depth(self, value):
        if self.depth_limit and self.depth_limit<value:
            control.AUV.depth(self, self.depth_limit)
            #getattr(self.ai, self.current_task_id).depthOverridden()
        else:
            control.AUV.depth(self, value)
    def prop(self, value):
        if self.prop_limit and self.prop_limit<value:
            control.AUV.prop(self, self.prop_limit)
            #getattr(self.ai, self.current_task_id).propOverridden()
        else:
            control.AUV.prop(self, value)
    def stop(self):
        #if the sub keeps turning too far, it might be an idea instead of calling
        #stop which disables auto pilots to set them to the current value
        self.prop(0)
        self.hbow(0)
        self.vbow(0)
        self.hstern(0)
        self.vstern(0)
        if self.bearing != None:
            self.bearing(self.current_bearing)
        self.pitch(0)

class auvControl(aiProcess):
    def __init__(self, opts):
        aiProcess.__init__(self, 'auv_control', opts.manager_id)
        self.auv = slightlyModifiedAUV(self.node)
        self.auv.depth_disabled = False
        
        self.control_state = {}

        self.current_task = None
        self.enabled = threading.Event() #stops the auv at the sending commands level

        self.depth_limit = None
        self.signal_msgs = Queue.Queue(5)
        self.processing_queue = Queue.Queue()
        
        #start receiving messages
        self._register()
        
    #SCRIPT COMMANDS
    @force_calling_process
    @external_function
    def auv_command(self, command, *args, **kwargs):
        task_id = kwargs.pop('calling_process')
        debug('auvControl::auv_command(self, calling_process=%s, cmd=%s, args=%s, kwargs=%s)' % (task_id, command, args, kwargs), 5)
        if self.enabled.is_set():
            if self.current_task == task_id:
                debug('Will call %s(*args, **kwargs)' % (getattr(self.auv, command)), 5)
                getattr(self.auv, command)(*args, **kwargs)
                if command == 'prop':
                    self.control_state['prop'] = args[0]
            else:
                warning('Script %s tried to move auv, but only script %s should be working' %(task_id, self.current_task))
        else:
            debug('Function not called as paused or disabled.', 5)
            
    #GENERAL FUNCTIONS (that could be called from anywhere)
    @external_function
    def stop(self):
        self.auv.stop()
        self.control_state = {}

    @external_function
    def lights_off(self):
        self.auv.downlights(0)
        self.auv.forwardlights(0)

    @external_function
    @event.event_func
    def signal(self, value):
        debug("Signal: {}".format(value))
        try:
            self.signal_msgs.put(value)
        except Queue.Full:
            error('Signalling queue is full, dropping request for signal')
    if False:
        #signaling thread
        def signal_loop(self):
            self.auv.forwardlights(0)
            while True:
                try:
                    msg = self.signal_msgs.get(block = False)
                except Queue.Empty:
                    break
                self.auv.forwardlights(0)
                time.sleep(2)
                for c in str(msg):
                    if not c in utils.morse.tab:
                        continue
                    for c2 in utils.morse.tab[c]:
                        self.auv.forwardlights(255)
                        if c2 == '-':
                            time.sleep(1)
                        else:
                            time.sleep(0.5)
                        self.auv.forwardlights(0)
                        time.sleep(0.5)
                    time.sleep(1)
    
    #TASK MANAGER COMMANDS
    @external_function
    @event.event_func
    def set_current_task_id(self, task_id):
        self.stop()
        self.current_task = task_id
        self.control_state = {}
        
    #OBSTACLE AVOIDANCE COMMANDS
    @external_function
    def enable(self):
        self.enabled.set()
    @external_function
    def disable(self):
        self.enabled.clear()
        self.auv.stop()
    @external_function
    def limit_depth(self, value):
        self.auv.depth_limit = value
    @external_function
    def limit_prop(self, value):
        self.auv.prop_limit = int(value) if int(value)<127 else 127
        try:
            if self.control_state['prop']>value: #_control_state[function][args/kwargs]
                self.auv.prop(int(value))
        except KeyError:
            #self.auv.prop(0)
            pass
                    
    def die(self):
        self.disable()
        aiProcess.die(self)

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    #p.add_argument('-d', '--disable_control', dest='disable_control', default=False,
    #action='store_true', help="stop AI script from controlling the sub")
    p.add_argument('-M', '--manager_id', dest='manager_id', default='',
                 action='store', help="id of relevent ai manager")
    opts, args = p.parse_known_args()
    ac = auvControl(opts)
    try:
        ac.run()
    finally:
        ac.die()
