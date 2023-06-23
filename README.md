# GifClipper
 A simple tool to trim, crop and export video files to animated gif.
  
  **Features:**
 - Drag and drop video files
 - Visually crop the video in the viewport
 - Visually trim the video on the timeline
 - Export to  animated GIF with settings for resolution and framerate

 **Tools:**
 - Qt 6.5.0
 - FFmpeg 5.1.2 (full build - GPL v3)

# Trim videos
 
![1](https://github.com/FunkyPizza/GifClipper/assets/31694150/19978374-5abb-4e83-9e66-b71b3ee93004)

 
# Crop videos
 
![2](https://github.com/FunkyPizza/GifClipper/assets/31694150/7adcbb46-9aa0-4a34-bb4c-e5f84f8e1f96)


# Settings & export to gif

![3](https://github.com/FunkyPizza/GifClipper/assets/31694150/96aa5f98-6cc5-4dbd-b45f-4afe36946e8d)



# Licence

Distributed under the GNU GPL V3.0 License. See `LICENSE.txt` for more information.

# Release log

**Release 0.1:**

This release contains all basic features in a working state:
- Import (drag&drop and file dialog selection)
- Basic timeline features (play/pause, backwards/forward, loop)
- Trim via the timeline
- Crop in a drag/select fashion directly in the viewport
- Export settings for resolution and fps

**Note:** Even though the required FFmpeg libraries are statically linked in this project, this release uses ffmpeg.exe instead.

