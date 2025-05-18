import socket
import numpy as np
import pyaudio
from scipy.signal import resample

UDP_PORT = 12334
MIC_SAMPLE_RATE = 16360
OUTPUT_RATE = 48000
CHANNELS = 2
DEVICE_NAME = "3DSMic"

p = pyaudio.PyAudio()

def find_output_device(name):
    for i in range(p.get_device_count()):
        dev = p.get_device_info_by_index(i)
        if name.lower() in dev['name'].lower() and dev['maxOutputChannels'] > 0:
            print(f"Using output device: {dev['name']} (Index: {i})")
            return i
    raise Exception(f"No output device containing '{name}' found.")

try:
    output_device_index = find_output_device(DEVICE_NAME)

    stream = p.open(format=pyaudio.paFloat32,
                    channels=CHANNELS,
                    rate=OUTPUT_RATE,
                    output=True,
                    output_device_index=output_device_index,
                    frames_per_buffer=1024)

    # --- INIT UDP SERVER ---
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", UDP_PORT))
    print(f"Listening on UDP port {UDP_PORT}.")

    # --- MAIN LOOP ---
    while True:
        print("1")
        data, addr = sock.recvfrom(2048)
        print("2")

        # 1. Convert 16-bit PCM mono to float32
        samples = np.frombuffer(data, dtype=np.int16).astype(np.float32) / 32768.0
        print("3")

        # 2. Resample to match output device
        resampled_len = int(len(samples) * OUTPUT_RATE / MIC_SAMPLE_RATE)
        resampled = resample(samples, resampled_len)

        print("4")

        # 3. Mono to stereo
        stereo = np.column_stack((resampled, resampled)).flatten()

        print("5")

        # 4. Play
        stream.write(stereo.tobytes())
        print("6")
        
        

except Exception as e:
    print("Error:", e)

finally:
    if 'stream' in locals():
        stream.stop_stream()
        stream.close()
    p.terminate()
    
