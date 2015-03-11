adb push inj /dev/
adb shell su chmod 777 /dev/inj
adb push inj_dalvik /dev/
adb shell su chmod 777 /dev/inj_dalvik
adb push libhook.so /dev/
adb shell su chmod 777 /dev/libhook.so

adb shell su ./dev/inj_dalvik com.example.socketclient
