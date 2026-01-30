#MyLinuxLearing
## 100ask imx6ull
KERN_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88

## qt configs
export QT_QPA_GENERIC_PLUGINS=tslib:/dev/input/event1
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0                       
export QT_QPA_FONTDIR=/usr/lib/fonts/
echo -e "\033[9;0]" > /dev/tty0
