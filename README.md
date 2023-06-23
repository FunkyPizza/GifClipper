# GifClipper
 A simple tool to trim, crop and export video files to animated gif.
  
 Features:
  - Drag and drop video files
 - Visually crop the video in the viewport
 - Visually trim the video on the timeline
 - Export to  animated GIF with settings for resolution and framerate

Using Qt 6.5.0
FFmpeg 5.1.2 (full build - GPL v3)

# Trim videos
 
![1](https://github.com/FunkyPizza/GifClipper/assets/31694150/05e08c3e-2e95-4cde-aef2-a434621b0ba2)
 
# Crop videos
 
![2](https://github.com/FunkyPizza/GifClipper/assets/31694150/b1698b44-6da4-45cc-b0e0-c96c3692249f)

# Settings & export to gif

![3](https://github.com/FunkyPizza/GifClipper/assets/31694150/2a1038a6-447d-4abe-b459-ed16af966e1c)


# Licence

Distributed under the GNU GPL V3.0 License. See `LICENSE.txt` for more information.

# Release log

Release 0.1:
This release contains all basic features in a working state:

Import (drag&drop and file dialog selection)
Basic timeline features (play/pause, backwards/forward, loop)
Trim via the timeline
Crop in a drag/select fashion directly in the viewport
Export settings for resolution and fps
Note: Even though the required FFmpeg libraries are statically linked in this project, this release uses ffmpeg.exe instead.

