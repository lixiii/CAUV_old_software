import time


def expWindow(n, alpha):
    t = 1
    r = [t]
    for i in xrange(0,n):
        r.append(t * alpha)
    return tuple(r)

class Controller:
    def __init__(self):
        pass
    def update(self, value):
        return 0

class PIDController(Controller):
    def __init__(self, (Kp, Ki, Kd), d_window = (1), err_clamp = 1e9):
        # the most recent end of the window is the start
        self.err = 0
        self.last_time = None
        self.derrs = []
        self.ierr = 0
        self.derr = 0
        self.derr_window = d_window
        self.err_clamp = err_clamp
        if abs(sum(d_window)) < 1e-30:
            raise Exception('Bad Window')
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
    
    def calcDErr(self):
        # apply self.derr_window over self.derrs
        renorm = sum(self.derr_window)
        keep_derrs = max(10, len(self.derr_window))
        self.derrs = self.derrs[-keep_derrs:]
        denormed = 0
        for i, derr in enumerate(self.derrs):
            if i == len(self.derr_window):
                break
            denormed += derr * self.derr_window[-i]
        return denormed / renorm
    
    def setKpid(self, (Kp, Ki, Kd)):
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        
    def update(self, err):
        now = time.time()
        last_err = self.err
        self.err = err
        if self.last_time is not None:
            self.derrs.append((self.err - last_err) / (now - self.last_time))
        else:
            self.derrs.append(0)
        self.derr = self.calcDErr()
        
        self.last_time = now
        
        self.ierr += self.err
        
        if self.ierr > self.err_clamp:
            self.ierr = self.err_clamp
        elif self.ierr < -self.err_clamp:
            self.ierr = -self.err_clamp
    
        return self.Kp * self.err +\
               self.Ki * self.ierr +\
               self.Kd * self.derr