import serial
import time
from pycaw.pycaw import AudioUtilities

# Get the default audio device and its volume 
device = AudioUtilities.GetSpeakers()
volume = device.EndpointVolume
last_volume = -1

ser = serial.Serial('COM3', 115200)

while True:
    current_volume = int(volume.GetMasterVolumeLevelScalar() * 100)  # Get current volume as a percentage

    if (current_volume - last_volume) >= 1:  # Only send if volume has changed by at least 1%
        print(f"Volume: {current_volume}%")
        ser.write(f"{current_volume}\n".encode())  # Send volume to STM32
        last_volume = current_volume

    time.sleep(0.1)  # Check volume every 100ms

