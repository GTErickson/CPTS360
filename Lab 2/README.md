# Lab2
Assignment page: https://eecs.wsu.edu/~cs360/LAB2.html
# Pointers
- use: './mk' to run your program
- use: './lab2.bin' to run KC's example.
- use: 'chmod +x mk' if you get permission errors

If KC's example program immediately segfaults. Then it's likely due to the PATH environment variable being too long which his program does not account for. To fix this, you can use 'PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin ./lab2.bin' to run his program. You will likely encounter this error if you are using WSL.
