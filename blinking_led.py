import time
import RPi.GPIO as GPIO# Import Raspberry Pi GPIO library

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(4, GPIO.OUT, initial=GPIO.LOW)

while True:
    GPIO.output(4,GPIO.HIGH)
    time.sleep(1)
    GPIO.output(4,GPIO.LOW)
    time.sleep(1)

