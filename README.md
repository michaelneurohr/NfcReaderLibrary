# NfcReaderlibrary

## Branch raspberrypi
Contains examples intended to be run on the Raspberry Pi using the EXPLORE-NFC [1] extension board.

Examples hosted in this branch are working with the Raspbian Jessie image.
All examples are using the NFC Reader Library as an external library. To build the library, please follow these steps:

1. Download SW282816 from the EXPLORE-NFC website
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
11. Create your own code
12. Compile your code with
   `gcc -o my-program my-code.c -Itypes -Iintfs -L/usr/lib -L/usr/local/lib -lwiringPi -lnxprdlib`
13. Run the program with
   `sudo ./my-program`