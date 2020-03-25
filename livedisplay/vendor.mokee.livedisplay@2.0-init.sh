#!/vendor/bin/sh

if [[ -f /data/vendor/display/mokee_color_profile ]]; then
    cat /data/vendor/display/mokee_color_profile > /sys/class/meizu/lcm/display/lut
else
    echo 1 > /sys/class/meizu/lcm/display/lut
fi
