import re

# Import the Aluminium library camera module.
import libal.camera

# Import Pillow if it's available.
try:
    import PIL
    import PIL.Image
except ImportError:
    PIL = None

# Open the first camera device (index 0).
camera = libal.camera.Camera(index=0, width=1280, height=720)

# Start reading frames.
camera.start()

# 📸 Read a frame in RGBA format, as a Python bytes object.
# The 'rgba_bytes' property will return None until the first frame is read.
rgba_bytes = b''
while len(rgba_bytes) == 0:
    rgba_bytes = camera.rgba_bytes

# Save the frame as an image.
if PIL:
    image = PIL.Image.frombytes(
        mode='RGBA',
        size=(camera.width, camera.height),
        data=rgba_bytes,
    )
    image.save(re.sub(r'.py$', r'.png', __file__))

# Stop reading frames.
camera.stop()
