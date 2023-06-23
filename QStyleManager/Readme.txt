Dark/Light styles taken from: https://github.com/ColinDuquesnoy/QDarkStyleSheet

To update: 
- Replace .qrc and .qss files.
- In QT Creator, change the .qrc files' "Prefix" to reflect their new directory (in QT Creator: Right Click file -> Change Prefix).
  Example: Set "/QStyleManager/dark" instead of "qdarkstyle/dark".
- If you're adding any new styles, check out QStyleManager.h to add/change style's path and enum 