import messaging
import threading
import time

class AUV(messaging.BufferedMessageObserver):
    def __init__(self, node):
        messaging.BufferedMessageObserver.__init__(self)
        self.__node = node
        node.join("control")
        node.addObserver(self)
        self.current_bearing = None
        self.current_depth = None
        self.current_pitch = None
        self.bearingCV = threading.Condition()
        self.depthCV = threading.Condition()
        self.pitchCV = threading.Condition()
        
        ## synchronising stuff
        #self.received_state_condition = threading.Condition()
        #self.received_state = None

    def send(self, msg):
        # send to control via self.__node
        self.__node.send(msg, "control")

    def stop(self):
        self.prop(0)
        self.hbow(0)
        self.vbow(0)
        self.hstern(0)
        self.vstern(0)
        self.bearing(None)
        self.pitch(None)
        self.depth(None)
    
    def getBearing(self):
        return self.current_bearing
    #def getBearing(self, timeout=3):
    #    self.received_state_condition.acquire()
    #    self.received_state = None
    #    self.send(messaging.StateRequestMessage())
    #    self.received_state_condition.wait(timeout)
    #    self.received_state_condition.release()
    #    return self.received_state.orientation

    def bearing(self, bearing):
        if bearing is not None:
            self.send(messaging.BearingAutopilotEnabledMessage(True, bearing))
        else:
            self.send(messaging.BearingAutopilotEnabledMessage(False, 0))

    def bearingAndWait(self, bearing, epsilon = 5, timeout = 30):
        startTime = time.clock()
        bearing(self, bearing)
        while min((bearing - current_bearing) % 360, (current_bearing - bearing) % 360) > epsilon and time.clock() - startTime < timeout:
            bearingCV.wait(timeout - time.clock() + startTime)
                
    def depth(self, depth):
        if depth is not None:
            self.send(messaging.DepthAutopilotEnabledMessage(True, depth))
        else:
            self.send(messaging.DepthAutopilotEnabledMessage(False, 0))

    def depthAndWait(self, depth, epsilon = 5, timeout = 30):
        startTime = time.clock()
        depth(self, depth)
        while abs(depth - current_depth) > epsilon and time.clock() - startTime < timeout:
            depthCV.wait(timeout - time.clock() + startTime)

    def pitch(self, pitch):
        if pitch is not None:
            self.send(messaging.PitchAutopilotEnabledMessage(True, pitch))
        else:
            self.send(messaging.PitchAutopilotEnabledMessage(False, 0))

    def pitchAndWait(self, pitch, epsilon = 5, timeout = 30):
        startTime = time.clock()
        pitch(self, pitch)
        while min((pitch - current_pitch) % 360, (current_pitch - pitch) % 360) > epsilon and time.clock() - startTime < timeout:
            pitchCV.wait(timeout - time.clock() + startTime)

    def bearingParams(self, kp, ki, kd, scale):
        self.send(messaging.BearingAutopilotParamsMessage(kp, ki, kd, scale))

    def depthParams(self, kp, ki, kd, scale):
        self.send(messaging.DepthAutopilotParamsMessage(kp, ki, kd, scale))

    def pitchParams(self, kp, ki, kd, scale):
        self.send(messaging.PitchAutopilotParamsMessage(kp, ki, kd, scale))

    def prop(self, value):
        self.checkRange(value)
        self.send(messaging.MotorMessage(messaging.MotorID.Prop, value))

    def hbow(self, value):
        self.checkRange(value)
        self.send(messaging.MotorMessage(messaging.MotorID.HBow, value))
   
    def vbow(self, value):
        self.checkRange(value)
        self.send(messaging.MotorMessage(messaging.MotorID.VBow, value))

    def hstern(self, value):
        self.checkRange(value)
        self.send(messaging.MotorMessage(messaging.MotorID.HStern, value))

    def vstern(self, value):
        self.checkRange(value)
        self.send(messaging.MotorMessage(messaging.MotorID.VStern, value))
    
    def motorMap(self, motor_id, zero_plus, zero_minus, max_plus = 127, max_minus = -127):
        #
        #        -127                           0                            127
        # demand:  |---------------------------|0|----------------------------| 
        # output:     |---------------|         0            |----------------|
        #             ^               ^                      ^                ^
        #             maxMinus     zeroMinus               zeroPlus        maxPlus
        #
        m = messaging.MotorMap()
        m.zeroPlus = zero_plus
        m.zeroMinus = zero_minus
        m.maxPlus = max_plus
        m.maxMinus = max_minus
        self.send(messaging.SetMotorMapMessage(motor_id, m))

    def propMap(self, zero_plus, zero_minus, max_plus = 127, max_minus = -127):
        # see doc for motorMap
        self.motorMap(messaging.MotorID.Prop, zero_plus, zero_minus, max_plus, max_minus)

    def hbowMap(self, zero_plus, zero_mbearinginus, max_plus = 127, max_minus = -127):
        # see doc for motorMap        
        self.motorMap(messaging.MotorID.HBow, zero_plus, zero_minus, max_plus, max_minus)

    def vbowMap(self, zero_plus, zero_minus, max_plus = 127, max_minus = -127):
        # see doc for motorMap        
        self.motorMap(messaging.MotorID.VBow, zero_plus, zero_minus, max_plus, max_minus)

    def vsternMap(self, zero_plus, zero_minus, max_plus = 127, max_minus = -127):
        # see doc for motorMap        
        self.motorMap(messaging.MotorID.VStern, zero_plus, zero_minus, max_plus, max_minus)

    def hsternMap(self, zero_plus, zero_minus, max_plus = 127, max_minus = -127):
        # see doc for motorMap        
        self.motorMap(messaging.MotorID.HStern, zero_plus, zero_minus, max_plus, max_minus)

    def v(self, value):
        self.vbow(value)
        self.vstern(value)

    def strafe(self, value):
        self.hbow(value)
        self.hstern(value)

    def r(self, value):
        self.hbow(value)
        self.hstern(-value)

    def checkRange(self, value):
        if value < -127 or value > 127:
            raise ValueError("invalid motor value: %d" % value)
    
    def onTelemetryMessage(self, m):
        #self.bearing = m.orientation.yaw
        self.current_bearing = m.orientation.yaw
        self.current_depth = m.depth
        self.current_pitch = m.orientation.pitch
        bearingCV.notifyAll()
        depthCV.notifyAll()
        pitchCV.notifyAll()

    ## synchronous-ifying stuff
    #def onStateMessage(self, m):
    #    self.received_state_condition.acquire()
    #    self.received_state = m
    #    self.received_state_condition.notify()
    #    self.received_state_condition.release()
