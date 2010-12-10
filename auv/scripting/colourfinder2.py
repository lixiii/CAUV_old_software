import cauv.messaging as messaging

import cauv
import cauv.control as control
import cauv.node

from movingaverage import MovingAverage

import time

class ColourFinder(messaging.BufferedMessageObserver):
    
   
    def __init__(self, node, bin, channel = 'Hue', no_trigger = 3):

        messaging.BufferedMessageObserver.__init__(self)
        self.__node = node
        node.join("processing")
        node.addObserver(self)
        self.bin = bin
        self.channel = channel
        self.no_trigger = no_trigger
        self.detect = 0
        self.binMovingmean = []
        for uni_bin in self.bin:
            self.binMovingmean.append(MovingAverage('upper', tolerance=0.15, maxcount=200, st_multiplier=2, st_on = 1))         #A class to calculate the moving average of last maxcount number of sample, mulitplier sets the tolerance range mulitplier and set trigger flag when sample is outside tolerance range

    def print_bins(self, m):
        #Routine to display all the bin values
        accum = 0
        for i, bin in enumerate(m.bins):
            accum += bin
            print 'bin %d: %f, accum: %f' %(i, bin, accum)    

    def onHistogramMessage(self, m):
        if m.type == self.channel:

            #self.print_bins(m)
            
            for j, uni_bin in enumerate(self.bin):

                self.binMovingmean[j].update(m.bins[uni_bin])
                #print 'bin %d: %f' %(uni_bin, m.bins[uni_bin])            
                
                if self.binMovingmean[j].trigger > self.no_trigger:
                    self.detect = 1
                    break
                else:
                    self.detect = 0
                  
                #print 'Count :', self.binMovingmean[j].count
                #print 'Moving average of bin %d is: %f' %(uni_bin, self.binMovingmean[j].movingMean)
                #print 'Standard error of bin %d is: %f' %(uni_bin, self.binMovingmean[j].movingError)

        #if self.detect==1:
            #print "Looks like we have found something!!!"            


if __name__ == '__main__':
    node = cauv.node.Node('ColFind')
    auv = control.AUV(node)
    detect = ColourFinder(node, [11, 12])
    while True:
        time.sleep(5)
