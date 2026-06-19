### Linux screenshot app Readme

### How to prepare:
```
sudo apt install libgtk-4-dev
sudo apt install libportal-dev xdg-desktop-portal xdg-desktop-portal-gnome
```



### Compile
```
gcc -Wall screenshot.c `pkg-config --cflags --libs gtk4 libportal` -o screenshot;
gcc -Wall screenshot_editor.c `pkg-config --cflags --libs gtk4 libportal` -lm -o screenshot_editor
``` 

### TODO

## Features
 - [ ] insertion of arrows
   - [x] button panel
   - [x] dot or arrow buttons
   - [x] arrow/figure struct, click logic
   - [ ] draw arrow
 - [ ] insertion of rectangles
 - [ ] layers, figures movement
 - [ ] button "save to buffer and exit"?
 - [ ] turn off form in screenshot.c 
 - [ ] create make file
 - [ ] Logging
 - [ ] increment new filenames

## Bug
 - [x] ubuntu shortcut doesn't open editor after form
 