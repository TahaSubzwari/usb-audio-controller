import serial
import serial.tools.list_ports
import time
import threading
import sys
import winreg
import comtypes

from pycaw.pycaw import AudioUtilities
from PIL import Image, ImageDraw
import pystray

# --- Config ---
BAUD_RATE = 115200
APP_NAME = "USB Audio Controller"
STARTUP_REG_KEY = r"Software\Microsoft\Windows\CurrentVersion\Run"

# --- Find STM32 COM port automatically ---
def find_stm32_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if port.vid == 0x0483 and port.pid == 0x374B:
            return port.device
    return None

# --- Registry startup ---
def set_startup(enable):
    exe_path = sys.executable if not getattr(sys, 'frozen', False) else sys.argv[0]
    key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, STARTUP_REG_KEY, 0, winreg.KEY_SET_VALUE)
    if enable:
        winreg.SetValueEx(key, APP_NAME, 0, winreg.REG_SZ, f'"{exe_path}"')
    else:
        try:
            winreg.DeleteValue(key, APP_NAME)
        except FileNotFoundError:
            pass
    winreg.CloseKey(key)

def is_startup_enabled():
    try:
        key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, STARTUP_REG_KEY, 0, winreg.KEY_READ)
        winreg.QueryValueEx(key, APP_NAME)
        winreg.CloseKey(key)
        return True
    except FileNotFoundError:
        return False

# --- Tray icon image ---
def create_tray_image():
    img = Image.new('RGB', (64, 64), color=(0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.ellipse([8, 8, 56, 56], outline=(255, 255, 255), width=4)
    draw.ellipse([20, 20, 44, 44], fill=(255, 255, 255))
    return img

# --- Stop signal ---
stop_event = threading.Event()

# --- Serial sender thread ---
def serial_thread():
    comtypes.CoInitialize()

    ser = None
    volume_obj = None

    try:
        device = AudioUtilities.GetSpeakers()
        volume_obj = device.EndpointVolume
    except Exception as e:
        print(f"Audio init failed: {e}")
        comtypes.CoUninitialize()
        return

    while not stop_event.is_set():
        try:
            if ser is None or not ser.is_open:
                port = find_stm32_port()
                if port:
                    ser = serial.Serial(port, BAUD_RATE, timeout=1)
                else:
                    time.sleep(2)
                    continue

            vol = round(volume_obj.GetMasterVolumeLevelScalar() * 100)
            ser.write(f"V{vol}\n".encode())
            time.sleep(0.02)

        except serial.SerialException:
            if ser:
                ser.close()
                ser = None
            time.sleep(2)
        except Exception as e:
            print(f"Error: {e}")
            time.sleep(1)

    comtypes.CoUninitialize()

# --- Tray menu actions ---
def on_startup_toggle(icon, item):
    enable = not is_startup_enabled()
    set_startup(enable)

def on_quit(icon, item):
    stop_event.set()
    icon.stop()

def build_menu():
    return pystray.Menu(
        pystray.MenuItem(
            "Run on startup",
            on_startup_toggle,
            checked=lambda item: is_startup_enabled()
        ),
        pystray.MenuItem("Quit", on_quit)
    )

# --- Main ---
def main():
    t = threading.Thread(target=serial_thread, daemon=True)
    t.start()

    icon = pystray.Icon(
        APP_NAME,
        create_tray_image(),
        APP_NAME,
        menu=build_menu()
    )
    icon.run()

if __name__ == "__main__":
    main()