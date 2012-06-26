#!/usr/bin/env python2.7
import cauv.node
from cauv import control, sonar
from cauv.debug import debug, warning, error, info

import time
import threading, Queue
import argparse
import utils.event as event
import utils.morse

from AI_classes import aiProcess, external_function, force_calling_process

#TODO basically the actual functionality of control, the ability to stop the sub, block script_ids etc

class slightlyModifiedAUV(control.AUV):
    def __init__(self, node):
        control.AUV.__init__(self, node)
        self.depth_limit = None
        self.prop_limit = None
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

class auvControl(aiProcess):
    def __init__(self, opts):
        aiProcess.__init__(self, 'auv_control')
        self.auv = slightlyModifiedAUV(self.node)
        self.auv.depth_disabled = False

        self.sonar = sonar.Sonar(self.node)
        self.active_tasks = {} #task_id: priority
        self.paused_tasks = []
        self.default_task = None
        self.waiting_for_control = {} #task_id: (priority, timeout)
        self.current_task = None
        self.enabled = threading.Event() #stops the auv at the sending commands level
        if not opts.disable_control:
            self.enabled.set()
        self.paused = False #stops the auv at script control level

        self.depth_limit = None
        self.signal_msgs = Queue.Queue(5)
        self.processing_queue = Queue.Queue()
        #values set by the current script
        self._control_state = {}
        #values set by the default script
        self._control_state_default = {}
        self._sonar_state_default = {}
        
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
                self._control_state[command] = (args, kwargs)
            else:
                warning('Script %s tried to move auv when script %s was in control \
                (scripts waiting for control were %s, scripts known to be running were %s)'
                %(task_id, self.current_task, self.waiting_for_control, self.active_tasks))
        else:
            debug('Function not called as paused or disabled.', 5)

    @force_calling_process
    @external_function
    def sonar_command(self, command, *args, **kwargs):
        task_id = kwargs.pop('calling_process')
        if self.enabled.is_set() and self.current_task == task_id:
            getattr(self.sonar, command)(*args, **kwargs)
            
    #If there are issues with scripts getting control, check here
    @event.event_func
    def control_timeout(self, process, time_out_at):
        if process in self.waiting_for_control and \
           self.waiting_for_control[process][1] == time_out_at:
            getattr(self.ai, process)._set_paused()
            getattr(self.ai, process).control_timed_out()
            self.waiting_for_control.pop(process)
        self.reevaluate()

    @external_function
    @event.event_func
    def request_control(self, timeout = None, calling_process = None,):
        try:
            time_out_at = timeout + time.time() if timeout else None
            self.waiting_for_control[calling_process] = (self.active_tasks[calling_process], time_out_at)
            if timeout is not None:
                self.control_timeout.trigger(delay = timeout, args = (calling_process, time_out_at))
        except KeyError:
            warning('Script %s wanted to take control of auv when not in list of allowed scripts \
            (scripts waiting for control were %s, scripts known to be running were %s)'
            %(calling_process, self.waiting_for_control, self.active_tasks))
            return
        if timeout:
            debug('Script %s has started waiting for control, with a %d timeout' %(calling_process, timeout))
        else:
            debug('Script %s has started waiting for control, with no timeout' %(calling_process))
        self.reevaluate()

    @external_function
    @event.event_func
    def drop_control(self, task_id):
        self.waiting_for_control.pop(task_id, None)
        #need to tell script its been paused, else if it wants control in the future it will think it already has it.
        getattr(self.ai, task_id)._set_paused()
        self.reevaluate()

    @external_function
    @event.event_func
    def pause(self):
        self.paused = True
        self.reevaluate()
        
    @external_function
    @event.event_func
    def resume(self):
        self.paused = False
        self.reevaluate()
        
    @external_function
    @event.event_func
    def pause_script(self, task_id):
        self.paused_tasks.append(task_id)
        self.reevaluate()
        
    @external_function
    @event.event_func
    def resume_script(self, task_id):
        self.paused_tasks.remove(task_id)
        self.reevaluate()
            
    #GENERAL FUNCTIONS (that could be called from anywhere)
    @external_function
    def stop(self):
        #if the sub keeps turning too far, it might be an idea instead of calling
        #stop which disables auto pilots to set them to the current value
        self.auv.prop(0)
        self.auv.hbow(0)
        self.auv.vbow(0)
        self.auv.hstern(0)
        self.auv.vstern(0)
        if self.auv.bearing != None:
            self.auv.bearing(self.auv.current_bearing)
        self.auv.pitch(0)
        #check that depth autopilot has been set,
        if 'depth' in self._control_state and self._control_state['depth'] != None:
            self.auv.depth(*self._control_state['depth'][0])

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
    def set_current_task_id(self, task_id, priority):
        try:
            self.waiting_for_control.pop(self.default_task)
        except KeyError:
            #presumably no task in the list
            pass
        #if currently in control, need to stop
        if self.current_task == self.default_task:
            self.stop()
        else:
            #else need to clear default script state so new script gets fresh state
            #except for depth
            if 'depth' in self._control_state_default:
                self._control_state_default = {'depth': self._control_state_default['depth']}
            else:
                self._control_state_default = {}
        self.default_task = task_id
        #Don't add to list if default set to none
        if task_id:
            self.waiting_for_control[task_id] = (priority, None)
        self.reevaluate()

    @external_function
    @event.event_func
    def add_additional_task_id(self, task_id, priority):
        self.active_tasks[task_id] = priority
        self.reevaluate()

    @external_function
    @event.event_func
    def remove_additional_task_id(self, task_id):
        self.active_tasks.pop(task_id)
        #try to remove from waiting for control list
        self.waiting_for_control.pop(task_id, None)
        getattr(self.ai, task_id)._set_paused()
        self.reevaluate()
        
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
        self.auv.prop_limit = value
        try:
            if self._control_state['prop'][0][0]>value: #_control_state[function][args/kwargs]
                self.auv.prop(value)
        except KeyError:
            self.auv.prop(0)
        
    def reevaluate(self):
        #find highest priority
        if not self.paused:
            try:
                highest_priority_task = max({task_id:y for task_id, y in self.waiting_for_control.iteritems() if not task_id in self.paused_tasks}.iteritems(),
                                            key=lambda x: x[1][0])[0]
            except ValueError:
                #list is empty
                highest_priority_task = None
        else:
            highest_priority_task = None
        if self.current_task == highest_priority_task:
            #no change, so don't do anything
            return
        #need to: store current values, set highest priority as current task, make sure scripts know whether they are in control
        #atm, just store values for default script TODO decide on a better way of doing this (or even if storing values is desirable)
        if self.current_task == self.default_task:
            self._sonar_state_default = self.sonar.__dict__.copy()
            self._control_state_default = self._control_state.copy()
        #stop current script and restore values
        self.current_task = None
        self.stop()
        if highest_priority_task == self.default_task:
            for command, (args, kwargs) in self._control_state_default.items():
                getattr(self.auv, command)(*args, **kwargs)
            self._control_state = self._control_state_default
            #restore sonar state
            self.sonar.__dict__.update(self._sonar_state_default)
            self.sonar.update()
        self.current_task = highest_priority_task
        for task_id in self.waiting_for_control:
            if task_id != self.current_task:
                getattr(self.ai, task_id)._set_paused()
        if self.current_task:
            getattr(self.ai, self.current_task)._set_unpaused()
                    
    def die(self):
        self.disable()
        aiProcess.die(self)

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('-d', '--disable_control', dest='disable_control', default=False,
                 action='store_true', help="stop AI script from controlling the sub")
    opts, args = p.parse_known_args()
    ac = auvControl(opts)
    try:
        ac.run()
    finally:
        ac.die()
