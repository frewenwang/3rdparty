# asan_symbolize.py
Memory problems occured very often and they are very easily neglected. Thanks to our great leader GM Gu, now we have a very useful tool to help us detect memory problem -- Address Santitizer. However, it can not been symbolized in android now. So we need to asan_symbolize.py to sybolized the result.

# how to use
1. make a new dir
2. make a new file and copy the asan report into it or simple pull out the orignal log
3. pull the related so/bin file
4. run the asan_symbolize.py

# nessary args
1. -c add you compile tool addr2line bin absolute path
2. -l / --logfile add your logfile
3. -a / --current_path add your current absolute path

#example
python asan_symbolize.py -c /home/mi/opt/android-ndk-r19c/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi- -l test_log3 --current_path /home/mi/Desktop/tmp/asan_test
