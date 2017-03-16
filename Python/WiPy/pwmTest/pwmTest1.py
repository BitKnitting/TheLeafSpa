from machine import Pin, Timer


def rising(arg):
    chrono.stop()
    p_in = Pin(Pin.exp_board.G10, mode=Pin.IN, pull=Pin.PULL_DOWN)
    if (th > 0):
        global tl
        tl = chrono.read_ms()
        p_in.callback(Pin.IRQ_FALLING, None)
        calcCO2()
    else: 
        p_in.callback(Pin.IRQ_FALLING, falling)
        chrono.start()

def falling(arg):
    chrono.stop()
    global th
    th = chrono.read_ms()
    p_in = Pin(Pin.exp_board.G10, mode=Pin.IN, pull=Pin.PULL_UP)
    p_in.callback(Pin.IRQ_RISING, rising)   
    chrono.start()
    
def calcCO2():    
    CO2 = 2000 * (th - 2 )/(th+tl - 4)
    print('\n------------------------------------------------') 
    print("CO2: ", CO2)
    print('------------------------------------------------') 
    
def doCO2Test():  
    global th 
    th = 0
    p_in = Pin(Pin.exp_board.G10, mode=Pin.IN, pull=Pin.PULL_UP)
    p_in.callback(Pin.IRQ_RISING, rising)
    
chrono = Timer.Chrono()
doCO2Test()

