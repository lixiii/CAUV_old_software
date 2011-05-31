import cauv.messaging as msg
import cauv.node as node

from cauv.debug import debug, info, warning, error

import shelve
import datetime
import sys
import math
import time
import cmd
import threading
import bisect

#TODO: there's quite a lot of code duplication here, we should make a CAUV
# python utility library with stuff like this in it

Datetime_Format = '%a %d %b %Y %H:%M:%S'

Ignore_Message_Attrs = (
    'group'
)

class YPRWrapper:
    def __init__(self, fypr = msg.floatYPR()):
        self.yaw = fypr.yaw
        self.pitch = fypr.pitch
        self.roll = fypr.roll
    def floatYPR(self):
        return msg.floatYPR(self.yaw, self.pitch, self.roll)

class SonarDataLineWrapper:
    def __init__(self, sdl = msg.SonarDataLine()):
        self.data = tuple(sdl.data)
        self.bearing = sdl.bearing
        self.bearingRange = sdl.bearingRange
        self.range = sdl.range
    def SonarDataLine(self):
        dl = msg.byteVec()
        for b in self.data:
            dl.append(b)
        return msg.SonarDataLine(dl, self.bearing, self.bearingRange, self.range)
 
class MotorIDWrapper:
    def __init__(self, mid = msg.MotorID()):
        self.value = int(mid)
    def MotorID(self):
        return msg.MotorID(self.value)

def dictFromMessage(message):
    attrs = message.__class__.__dict__
    r = {}
    for k in attrs:
        if not k.startswith('__') and not k in Ignore_Message_Attrs:
            # hack hack hack hack hack
            # I'm sure there is a simpler way of doing this, but m.__dict__ is
            # unhelpfully empty...
            r[k] = attrs[k].__get__(message)
            if type(r[k]) == type(msg.floatYPR()):
                r[k] = YPRWrapper(r[k])
            elif type(r[k]) == type(msg.SonarDataLine()):
                r[k] = SonarDataLineWrapper(r[k])
            elif type(r[k]) == type(msg.MotorID()):
                r[k] = MotorIDWrapper(r[k])
    r['__message_name__'] = message.__class__.__name__
    return r

def dictToMessage(attr_dict):
    new_attrs = {}
    for k in attr_dict:
        if k.startswith('__'):
            continue
        if attr_dict[k].__class__ == SonarDataLineWrapper:
            new_attrs[k] = attr_dict[k].SonarDataLine()
        elif attr_dict[k].__class__ == YPRWrapper:
            new_attrs[k] = attr_dict[k].floatYPR()
        elif attr_dict[k].__class__ == MotorIDWrapper:
            new_attrs[k] = attr_dict[k].MotorID()
        else:
            new_attrs[k] = attr_dict[k]
    return getattr(msg, attr_dict['__message_name__'])(**new_attrs)

def incFloat(f):
    if f == 0.0:
        return sys.float_info.min
    m, e = math.frexp(f)
    return math.ldexp(m + sys.float_info.epsilon / 2, e)


class MessageLoggerComment:
    def __init__(self, comment_str):
        self.datetime = datetime.datetime.now()
        self.comment = comment_str

    def __repr__(self):
        return 'Comment: %s (recorded at %s)' % (
            self.comment, self.datetime.strftime(Datetime_Format)
        )


class MessageLoggerSession:
    def __init__(self, info_str):
        self.datetime = datetime.datetime.now()
        self.info = info_str

    def __repr__(self):
        return 'New session (%s), recorded at %s' % (
            self.info, self.datetime.strftime(Datetime_Format)
        )


class Logger(msg.MessageObserver):
    def __init__(self, cauv_node, shelf_fname, do_record, playback_rate = 1.0):
        msg.MessageObserver.__init__(self)
        self.node = cauv_node
        self.__recording = False
        self.__shelf = None
        self.__cached_keys = set()
        self.__shelf_fname = None # set by setShelf
        self.__tzero = time.time()
        self.__playback_lock = threading.Lock()
        self.__playback_active = False
        self.__playback_finished = threading.Condition()
        self.__playback_thread = None
        self.__playback_rate = playback_rate
        self.__playback_start_time = 0
        self.setShelf(shelf_fname)
        self.doRecord(do_record)

    def close(self):
        'This method MUST BE CALLED before destroying the class'
        self.doRecord(False)
        self.stopPlayback()
        self.__shelf.close()
        self.__shelf = None
        self.__cached_keys = None

    def playbackIsActive(self):
        self.__playback_lock.acquire()
        r =  self.__playback_active
        self.__playback_lock.release()
    
    def setPlaybackRate(self, rate):
        # messages are played back at rate * real time (higher is faster)
        self.__playback_rate = rate

    def startPlayback(self, start_time):
        if self.__playback_active:
            error("can't start playback: playback is already active")
            return
        self.__playback_lock.acquire()
        self.__playback_start_time = start_time
        self.__playback_active = True
        self.__playback_lock.release()
        self.__playback_thread = threading.Thread(target=self.playbackRunloop)
        self.__playback_thread.start()

    def stopPlayback(self):
        if not self.__playback_active:
            return
        self.__playback_lock.acquire()
        self.__playback_active = False
        self.__playback_lock.release()
        self.__playback_finished.acquire()
        self.__playback_finished.wait()
        self.__playback_finished.release()
        self.__playback_thead = None

    def __playbackHasBeenStopped(self):
        r = False
        self.__playback_lock.acquire()
        if not self.__playback_active:
            r = True
        self.__playback_lock.release()
        return r

    def playbackRunloop(self):
        debug('playback started')
        sorted_keys = sorted(list(self.__cached_keys))
        start_idx = bisect.bisect_left(sorted_keys, self.__playback_start_time)
        sorted_keys = sorted_keys[start_idx:]
        if not len(sorted_keys):
            return
        tstart_recorded = sorted_keys[0]
        tstart_playback = self.relativeTime()
        try:
            for i in xrange(0, len(sorted_keys)):
                if self.__playbackHasBeenStopped():
                    info('playback stopped')
                    break
                next_thing = self.__shelf[sorted_keys[i].hex()]
                if type(next_thing) == type(dict()):
                    # this is a message
                    m = dictToMessage(next_thing)
                    debug('t=%g, sending: %s' % (sorted_keys[i], m), 5)
                    self.node.send(m)
                else:
                    info('playback: %s' % next_thing)
                if i != len(sorted_keys) - 1:
                    next_time = sorted_keys[i+1]
                    time_to_sleep_for = (next_time - tstart_recorded) -\
                                        self.__playback_rate * (self.relativeTime() - tstart_playback)
                    #debug('sleeping for %gs' % time_to_sleep_for, 5)
                    if time_to_sleep_for/self.__playback_rate > 10:
                        warning('more than 10 seconds until next message will be sent (%gs)' %
                              (time_to_sleep_for/self.__playback_rate))
                    while time_to_sleep_for > 0 and not self.__playbackHasBeenStopped():
                        sleep_step = min((time_to_sleep_for/self.__playback_rate, 0.2))
                        time_to_sleep_for -= sleep_step*self.__playback_rate
                        time.sleep(sleep_step)
            if i == len(sorted_keys)-1:
                info('playback complete')
        except Exception, e:
            error('error in playback: ' + str(e))
            raise
        finally:
            self.__playback_finished.acquire()
            self.__playback_finished.notify()
            self.__playback_finished.release()
            self.__playback_lock.acquire()
            self.__playback_active = False
            self.__playback_lock.release()
        debug('playback finished')

    def setShelf(self, fname):
        if self.__shelf is not None and self.__shelf_fname != fname:
            self.stopPlayback()
            old_shelf = self.__shelf()
            self.__shelf = shelve.open(fname)
            old_shelf.close()
        elif self.__shelf is None:
            self.__shelf = shelve.open(fname)
        if self.__shelf is None or self.__shelf_fname != fname:
            self.__shelf_fname = fname
            # slow!
            self.__cached_keys = set(map(float.fromhex, self.__shelf.keys()))
            if len(self.__cached_keys) > 0:
                # add our messages on at the end
                debug('selected shelf already exists: appending at end')
                self.__tzero = time.time() - (max(self.__cached_keys) + 1)
            self.shelveObject(MessageLoggerSession(fname))
            self.onNewSession()

    def onNewSession(self):
        # derived classes can override this
        pass

    def addComment(self, comment_str):
        info('recoding comment "%s"' % comment_str)
        self.shelveObject(MessageLoggerComment(comment_str))

    def doRecord(self, do_record):
        if self.__recording == do_record:
            return
        if do_record == True:
            self.stopPlayback()
            self.node.addObserver(self)
        else:
            self.node.removeObserver(self)
            self.__shelf.sync()

    def relativeTime(self):
        return time.time() - self.__tzero

    def shelveObject(self, d):
        t = self.relativeTime()
        debug('shelving object: t=%g' % t, 6)
        while t in self.__cached_keys:
            t = incFloat(t)
        self.__shelf[t.hex()] = d
        self.__cached_keys.add(t)

    def shelveMessage(self, m):
        mdict = dictFromMessage(m)
        self.shelveObject(mdict)


class CmdPrompt(cmd.Cmd):
    def __init__(self, msg_logger, name):
        self.ml = msg_logger
        self.name = name
        cmd.Cmd.__init__(self)
        self.ruler = ''
        self.undoc_header = 'other commands:'
        self.doc_header = 'available commands:'
        self.setPrompt()
        self.intro = '''
%s, available commands:
    "filename FILENAME" - close the current shelf and use FILENAME as the shelf from now on
    "stop"              - stop recording or playback
    "record"            - stop playback and start recording data
    "playback [RATE] [START_TIME]"
                        - stop recording and start playing back recorded data
                          at RATE * real time, starting at START_TIME into
                          the recoded data
    "c COMMENT STRING"  - record COMMENT STRING in the shelf
''' % self.name
    def setPrompt(self):
        if self.ml.playbackIsActive():
            self.prompt = '<< '
        else:
            self.prompt = '>> '
    def do_filename(self, filename):
        '"filename FILENAME" - close the current shelf and use FILENAME as the shelf from now on'
        try:
            self.ml.setShelf(filename)
        except:
            print 'invalid filename "%s"' % filename
        self.setPrompt()
    def complete_filename(self, filename):
        # TODO: filename completions
        return []
    def do_stop(self, l):
        '"stop"              - stop recording or playback'
        if len(l):
            print '"stop" takes no arguments'
            return
        self.ml.stopPlayback()
        self.ml.doRecord(False)
        self.setPrompt()
    def do_record(self, l):
        '"record"            - stop playback and start recording data'
        if len(l):
            print '"record" takes no arguments'
            return
        self.ml.doRecord(True)
        self.setPrompt()
    def do_playback(self, l):
        '''"playback [RATE] [START_TIME]"
                        - stop recording and start playing back recorded data
                          at RATE * real time, starting at START_TIME into
                          the recoded data'''

        rate, start = l.split()
        start_time = 0.0
        if len(rate):
            try:
                self.ml.setPlaybackRate(float(rate))
            except ValueError, e:
                print 'could not set playback rate: "%s" (value should be a number)' % rate
                return
        if len(start):
            try:
                start_time = float(start)
            except ValueError, e:
                print 'could not set start time: "%s" (value should be a number of seconds)' % start
                return
        self.ml.startPlayback(start_time)
        self.setPrompt()
    def do_c(self, line):
        '"c COMMENT STRING"  - record COMMENT STRING in the shelf'
        self.ml.addComment(line)
    def do_EOF(self, line):
        return True
    def emptyline(self):
        self.onecmd('help')
        return False