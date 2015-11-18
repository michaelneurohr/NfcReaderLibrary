The example project is intended to be used with the NFC Reader Library directly.
It does not make use of Neard and NeardAL.
Before running it, please make sure that the daemon neard-explorenfc is not running.

Follow these steps to compile and run the example.

1. Download Neard EXPLORE-NFC Dev (SW282816) from the [EXPLORE-NFC documents website](http://www.nxp.com/board/PNEV512R.html#documentation)
2. Extract the file with the command
   `unzip SW282816.zip`
3. Extract the sources with the command
   `tar -xvf neard-explorenfc_0.9.orig.tar.gz`
4. Install dependencies:
   `sudo apt-get install wiringpi autoconf libtool libglib2.0-dev`
5. Install NeardAL development libraries
   `sudo dpkg -i libneardal-dev_0.14.2-1_armhf.deb`
6. Enter the directory neard-explorenfc-0.9 with
   `cd neard-explorenfc-0.9`
7. Run the command
   `./bootstrap`
8. Export environment variables for wiringPi with the commands
   `export WIRINGPI_CFLAGS=-I/usr/include`
   `export WIRINGPI_LIBS=-L/usr/lib\ -lwiringPi`
8. Run
   `./configure`
9. Finally compile with
   `make`
10. Copy the resulting library with
   `sudo cp nxprdlib/libnxprdlib.a /usr/local/lib/`
11. Compile the example with
   `gcc -o read-hce read-hce.c -Itypes -Iintfs -L/usr/lib -L/usr/local/lib -lwiringPi -lnxprdlib`
12. Run the program with
   `sudo ./read-hce`