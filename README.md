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
