Linux screenshot app Readme


### How to prepare:
```
sudo apt install libgtk-4-dev
sudo apt install libportal-dev xdg-desktop-portal xdg-desktop-portal-gnome
```



### Compile
```
gcc -Wall screenshot.c `pkg-config --cflags --libs gtk4 libportal` -o screenshot;
gcc -Wall screenshot_editor.c `pkg-config --cflags --libs gtk4 libportal` -o screenshot_editor
``` 

### TODO

 - [ ] insertion of arrows
 - [ ] insertion of rectangles
 - [ ] button "save to buffer and exit"
 - [ ] turn off form in screenshot.c 
 - [ ] create make file
