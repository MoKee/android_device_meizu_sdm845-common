#!/vendor/bin/sh

if [[ ! -f /data/vendor/display/mokee_color_profile ]]; then
    echo 1 > /data/vendor/display/mokee_color_profile
fi

cat /data/vendor/display/mokee_color_profile > /sys/class/meizu/lcm/display/lut
