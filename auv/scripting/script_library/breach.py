from AI_classes import aiScript, aiScriptOptions, aiScriptState
from cauv.debug import debug, warning, error, info

import time

class scriptOptions(aiScriptOptions):
    depth = 4
    angle = 45
    prop = 127
    bearing = 260
    forward_time = 10
    vert_prop = 127
    
class scriptState(aiScriptState):
    already_run = False

class script(aiScript):
    def run(self):
        if self.persist.already_run:
            return
        self.persist.already_run = True
        self.bearing(self.options.bearing)
        self.depthAndWait(self.options.depth)
        self.pitch(self.options.angle)
        self.depth(None)
        self.prop(self.options.prop)
        time.sleep(self.options.forward_time)
        self.pitch(0)
        self.prop(0)
        #attempt 2
        self.depthAndWait(self.options.depth)
        self.depth(None)
        self.pitch(None)
        self.prop(self.options.prop)
        self.vbow(self.options.vert_prop)
        self.vstern(self.options.vert_prop)
        time.sleep(self.options.forward_time)
        self.vbow(0)
        self.vstern(0)
        self.prop(0)
        self.stop()
        
        