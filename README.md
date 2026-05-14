Linux screenshot app Readme


### How to prepare:
```
sudo apt install libgtk-4-dev
sudo apt install libportal-dev xdg-desktop-portal xdg-desktop-portal-gnome
```



### Compile
```
gcc -Wall main.c `pkg-config --cflags --libs gtk4`;./a.out
``` 
