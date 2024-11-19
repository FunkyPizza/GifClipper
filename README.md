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
 
![1](https://github.com/user-attachments/assets/fd78b12f-8290-4822-969b-cf88784c2610)

 
# Crop videos
 
![2](https://github.com/user-attachments/assets/6282fec2-24e9-4ba0-8f06-24cd2a7fbd29)


# Settings & export to gif

![3](https://github.com/user-attachments/assets/da8cdb24-250a-4682-a95a-0cc5f5d9d853)


# Licence

Distributed under the GNU GPL V3.0 License. See `LICENSE.txt` for more information.

# Release log
**Release 0.4**

- Fixed issues with video resolution parameter was not properly applied if no crop was applied.
- Fixed issues with missing FFMPEG dependency dlls.
- Improved Quality settings (the 'Low' quality settings will now create files notably smaller with little loss).
- Fully implemented transcoders are now available to use as an alternative of relying on ffmpeg.exe commands. Both types of transcoders should produce very similar results. See 'Use CMD transcoder' tickbox to enable or disable the use of CMD transcoders.

**Note:** In this release, the MP4 transcoder is notably slower than its CMD counterpart, the quality of the output file is not affected.
__________
**Release 0.3**

- GifClipper can now export the current edit as a MP4 video (all Export Settings still apply)
- Added 'Quality' export settings to create smaller but less pretty exports (old gif export settings are equivalent to the current 'High')

**Note:** This release uses ffmpeg.exe.
__________
**Release 0.2**

- Fixed issue where GifClipper could use any FFMPEG executable available on the running machine.
- FFMPEG will now run without showing a window
- Fixed issue where the status bar would not display information during conversion.


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

