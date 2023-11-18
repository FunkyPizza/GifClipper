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
 
![1](https://github.com/FunkyPizza/GifClipper/assets/31694150/e5f15211-2b44-42c0-b7ad-dca34f4c0ba3)

 
# Crop videos
 
 ![2](https://github.com/FunkyPizza/GifClipper/assets/31694150/8e5eeeac-7eae-4de7-8575-d1de50f8fc6b)


# Settings & export to gif

![3](https://github.com/FunkyPizza/GifClipper/assets/31694150/2026685b-320e-4c69-91fa-0184d2b03390)


# Licence

Distributed under the GNU GPL V3.0 License. See `LICENSE.txt` for more information.

# Release log

**GifClipper - Release 0.2**

Fixed issue where GifClipper could use any FFMPEG executable available on the running machine.
FFMPEG will now run without showing a window
Fixed issue where the status bar would not display information during conversion.

**Note:** This release uses ffmpeg.exe.
__________
**Release 0.1:**

This release contains all basic features in a working state:
- Import (drag&drop and file dialog selection)
- Basic timeline features (play/pause, backwards/forward, loop)
- Trim via the timeline
- Crop in a drag/select fashion directly in the viewport
- Export settings for resolution and fps

**Note:** Even though the required FFmpeg libraries are statically linked in this project, this release uses ffmpeg.exe instead.

