# hell yeah artificial intelligence

import pyaudio
import numpy as np

# Configuration
fs = 48000          # Sample rate
duration = 5        # seconds
freq = 440.0        # A4 tone
channels = 2        # Stereo

# Generate tone
samples = (0.2 * np.sin(2 * np.pi * np.arange(fs * duration) * freq / fs)).astype(np.float32)

print(samples)
# Duplicate to stereo
stereo_samples = np.column_stack((samples, samples)).flatten()

# Initialize PyAudio
p = pyaudio.PyAudio()

# Find output device (e.g., "VirtualMic")
def find_output_device(name):
    for i in range(p.get_device_count()):
        dev = p.get_device_info_by_index(i)
        if name.lower() in dev['name'].lower() and dev['maxOutputChannels'] > 0:
            print(f"Using output device: {dev['name']} (Index: {i})")
            return i
    raise Exception(f"No output device containing '{name}' found.")

try:
    output_device_index = find_output_device("VirtualMic")

    # Open stream
    stream = p.open(format=pyaudio.paFloat32,
                    channels=channels,
                    rate=fs,
                    output=True,
                    output_device_index=output_device_index,
                    frames_per_buffer=1024)

    # Write to stream
    stream.write(stereo_samples.tobytes())

    # Clean up
    stream.stop_stream()
    stream.close()
    p.terminate()

except Exception as e:
    print("Error:", e)
