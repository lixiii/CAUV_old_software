#!/usr/bin/env python

import cauv
import cauv.messaging as msg
from cauv.debug import debug, info, warning, error

from AI_classes import aiScript, aiScriptOptions

import time
import math
import traceback

from utils.control import expWindow, PIDController

class scriptOptions(aiScriptOptions):
    Do_Prop_Limit = 10  # max prop for forward/backward adjustment
    Camera_FOV = 60     # degrees
    Warn_Seconds_Between_Sights = 5
    Give_Up_Seconds_Between_Sights = 30
    Node_Name = "py-CrcB" # unused
    Depth_Ceiling = 0.8
    Strafe_Speed = 20   # (int [-127,127]) controls strafe speed
    Buoy_Size = 0.15     # (float [0.0, 1.0]) controls distance from buoy. Units are field of view (fraction) that the buoy should fill
    Size_Control_kPID = (-30, 0, 0)  # (Kp, Ki, Kd)
    Size_DError_Window = expWindow(5, 0.6)
    Size_Error_Clamp = 1e30
    Angle_Control_kPID = (0.6, 0, 0) # (Kp, Ki, Kd)
    Angle_DError_Window = expWindow(5, 0.6)
    Angle_Error_Clamp = 1e30
    Depth_Control_kPID = (-0.05, -0.01, 0) # (Kp, Ki, Kd)
    Depth_DError_Window = expWindow(5, 0.6)
    Depth_Error_Clamp = 200
    
    Pipeline_File = 'circle_buoy.pipe'

    class Meta:
        dynamic = [
            'Do_Prop_Limit', 'Strafe_Speed', 'Buoy_Size',
            'Size_Control_kPID',  'Size_DError_Window',  'Size_Error_Clamp',
            'Angle_Control_kPID', 'Angle_DError_Window', 'Angle_Error_Clamp',
            'Depth_Control_kPID', 'Depth_DError_Window', 'Depth_Error_Clamp'
        ]


class script(aiScript):
    def __init__(self, script_name, opts):
        aiScript.__init__(self, script_name, opts)
        # self.node is set by aiProcess (base class of aiScript)
        self.node.join('processing')
        self.__strafe_speed = self.options.Strafe_Speed
        self.__buoy_size = self.options.Buoy_Size
        self.size_pid = PIDController(
                self.options.Size_Control_kPID,
                self.options.Size_DError_Window,
                self.options.Size_Error_Clamp
        )
        self.angle_pid = PIDController(
                self.options.Angle_Control_kPID,
                self.options.Angle_DError_Window,
                self.options.Angle_Error_Clamp
        )
        self.depth_pid = PIDController(
                self.options.Depth_Control_kPID,
                self.options.Depth_DError_Window,
                self.options.Depth_Error_Clamp
        )
                
        self.time_last_seen = None
    
    def reloadOptions(self):
        self.__strafe_speed = self.options.Strafe_Speed
        self.__buoy_size = self.options.Buoy_Size
        self.size_pid.setKpid(self.options.Size_Control_kPID)
        self.size_pid.derr_window = self.options.Size_DError_Window
        self.size_pid.err_clamp   = self.options.Size_Error_Clamp
        self.angle_pid.setKpid(self.options.Angle_Control_kPID)
        self.angle_pid.derr_window = self.options.Angle_DError_Window
        self.angle_pid.err_clamp   = self.options.Angle_Error_Clamp
        self.angle_pid.setKpid(self.options.Depth_Control_kPID)
        self.angle_pid.derr_window = self.options.Depth_DError_Window
        self.angle_pid.err_clamp   = self.options.Depth_Error_Clamp

    def optionChanged(self, option_name):
        info('notified that %s changed' % str(option_name))
        self.reloadOptions()
    
    def telemetry(self, name, value):
        self.node.send(msg.GraphableMessage(name, value))

    def onCirclesMessage(self, m):
        if str(m.name) == 'buoy':
            debug('Received circles message: %s' % str(m), 4)
            mean_circle_position = [0.5, 0.5]
            mean_circle_radius = 1
            num_circles = len(m.circles)
            if num_circles != 0:
                mean_circle_position = [0,0]
                mean_circle_radius = 0
                for circle in m.circles:
                    mean_circle_position[0] += circle.centre.x
                    mean_circle_position[1] += circle.centre.y
                    mean_circle_radius += circle.radius
                mean_circle_position[0] /= num_circles
                mean_circle_position[1] /= num_circles
                mean_circle_radius /= num_circles
                debug('Buoy at %s %s' % (mean_circle_position,
                                         mean_circle_radius))
                self.actOnBuoy(mean_circle_position,
                               mean_circle_radius)
            else:
                debug('No circles!', 3)
        else:
            debug('Ignoring circles message: %s' % str(m))
    
    def getBearingNotNone(self):
        b = self.auv.getBearing()
        if b is None:
            warning('no current bearing available')
            b = 0
        return b
    
    def getDepthNotNone(self):
        d = self.auv.current_depth
        if d is None:
            warning('no current depth available')
            d = 0
        return d

    def actOnBuoy(self, centre, radius):
        now = time.time()
        if self.time_last_seen is not None and \
           now - self.time_last_seen > self.options.Warn_Seconds_Between_Sights:
            info('picked up buoy again')
             
        pos_err_x = centre[0] - 0.5
        pos_err_y = centre[1] - 0.5
        if pos_err_x < -0.5 or pos_err_x > 0.5: warning('buoy outside image (x): %g' % pos_err_x)
        if pos_err_y < -0.5 or pos_err_y > 0.5: warning('buoy outside image (y): %g' % pos_err_y)
        #
        #          FOV /     - 0.5
        #             /      |
        #            /       |
        #           /        |
        #          /  buoy   |
        #         /    o     - pos_err
        #        /   /       |
        #       /  /         |
        #      / /           |
        #     // ) angle_err |
        # |->/- - - - -      -  0
        #         
        #    |<------------->|
        #       = 0.5 / sin(FOV/2) = plane_dist
        #
        plane_dist = 0.5 / math.sin(math.radians(self.options.Camera_FOV)/2.0)
        angle_err = math.degrees(math.asin(pos_err_x / plane_dist)) 
        depth_err = pos_err_y
        
        turn_to = self.getBearingNotNone() + self.angle_pid.update(angle_err)
        self.auv.bearing(turn_to)

        depth_to = self.getDepthNotNone() + self.depth_pid.update(depth_err)
        if depth_to < self.options.Depth_Ceiling:
            warning('setting depth limited by ceiling at %gm' %
                self.options.Depth_Ceiling
            )
            depth_to = self.options.Depth_Ceiling
        self.auv.depth(depth_to)

        size_err = radius - self.__buoy_size
        do_prop = self.size_pid.update(size_err)

        if do_prop > self.options.Do_Prop_Limit:
            do_prop = self.options.Do_Prop_Limit
        if do_prop < -self.options.Do_Prop_Limit:
            do_prop = -self.options.Do_Prop_Limit
        self.auv.prop(int(round(do_prop)))
        self.time_last_seen = now

        debug('angle (e=%.3g, ie=%.3g de=%.3g) size (e=%.3g, ie=%.3g, de=%.3g), depth (e=%.3g, ie=%.3g de=%.3g) current_deppth=%g' % (
            self.angle_pid.err, self.angle_pid.ierr, self.angle_pid.derr,
            self.size_pid.err, self.size_pid.ierr, self.size_pid.derr,
            self.depth_pid.err, self.depth_pid.ierr, self.depth_pid.derr,
            self.getDepthNotNone()
        ))
        debug('turn to %g, prop to %s, dive to %g' % (turn_to, do_prop, depth_to))
        
        # Telemetry: TODO: probably move message generation to pid class, also
        # would be nice if GraphableMessages accepted a list of things to plot,
        # not just one
        def pidTelem(pid,name):
            self.telemetry('%s-err' % name, pid.err)
            self.telemetry('%s-derr' % name, pid.derr)
            self.telemetry('%s-ierr' % name, pid.ierr)
        pidTelem(self.angle_pid,'BuoyAngle')
        pidTelem(self.size_pid,'BuoySize')
        pidTelem(self.depth_pid,'BuoyDepth')

    def run(self):
        #self.request_pl(self.options.Pipeline_File)
        start_bearing = self.auv.getBearing()
        entered_quarters = [False, False, False, False]
        exit_status = 'SUCCESS'
        info('Waiting for circles...')
        try:
            while False in entered_quarters:
                self.auv.strafe(self.__strafe_speed)
                time.sleep(0.5)
                time_since_seen = 0
                if self.time_last_seen is not None:
                    time_since_seen = time.time() - self.time_last_seen
                    if time_since_seen > self.options.Warn_Seconds_Between_Sights:
                        warning('cannot see buoy: last seen %g seconds ago' %
                                time_since_seen)
                        self.auv.strafe(0)
                    else:
                        self.auv.strafe(self.__strafe_speed)
                    if time_since_seen > self.options.Give_Up_Seconds_Between_Sights:
                        self.log('Buoy Circling: lost sight of the buoy!')
                        raise Exception('lost the buoy: giving up!')
                if self.auv.getBearing() > -180 and self.auv.getBearing() < -90:
                    entered_quarters[3] = True
                if self.auv.getBearing() > -90 and self.auv.getBearing() < 0:
                    entered_quarters[2] = True
                if self.auv.getBearing() > 0 and self.auv.getBearing() < 90:
                    entered_quarters[0] = True
                if self.auv.getBearing() > 90 and self.auv.getBearing() < 180:
                    entered_quarters[1] = True
                if self.auv.getBearing() > 180 and self.auv.getBearing() < 270:
                    entered_quarters[2] = True
                if self.auv.getBearing() > 270 and self.auv.getBearing() < 360:
                    entered_quarters[3] = True
            while self.auv.getBearing() < start_bearing:
                info('Waiting for final completion...')
                time.sleep(0.5)
            self.log('Buoy Circling: completed successfully')
            self.log('Attempting to cut the buoy free...')
            self.auv.bearing(self.auv.getBearing())
            old_depth = self.auv.getDepth()
            self.auv.depth(old_depth + 0.8)
            self.auv.prop(80)
            self.auv.cut(1)
            ts = time.time()
            while time.time() - ts < self.options.Cut_Time:
                time.sleep(0.5)
            if old_depth < self.options.Depth_Ceiling:
                old_depth = self.options.Depth_Ceiling
            self.log('Cutting complete!')
            self.auv.depth(old_depth)
            self.auv.cut(0)
            self.auv.prop(0)
        except:
            exit_status = 'FAIL'
            error(traceback.format_exc())
        finally:
            info('Dropping Pipeline...')
            self.drop_pl(self.options.Pipeline_File)
            info('Stopping...')
            self.auv.stop()
        info('Complete!')
        self.notify_exit(exit_status)

