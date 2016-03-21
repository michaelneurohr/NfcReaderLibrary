# NfcReaderlibrary

## Branch raspberrypi
Contains examples intended to be run on the Raspberry Pi using the [EXPLORE-NFC](http://www.nxp.com/board/PNEV512R.html) extension board.

Examples hosted in this branch are working with the Raspbian Jessie image.
All examples are using the NFC Reader Library as an external library. To build the library, please follow these steps:

1. Download Neard EXPLORE-NFC Dev (SW282816) from the [EXPLORE-NFC documents website](http://www.nxp.com/board/PNEV512R.html#documentation)
2. Extract the file with the command
   `unzip SW282816.zip`
3. Extract the sources with the command
   `tar -xvf neard-explorenfc_0.9.orig.tar.gz`
4. Install dependencies:
   `sudo apt-get install wiringpi autoconf libtool libglib2.0-dev`
5. Enter the directory neard-explorenfc-0.9 with
   `cd neard-explorenfc-0.9`
6. Run the command
   `./bootstrap`
7. Export environment variables for wiringPi with the commands
   `export WIRINGPI_CFLAGS=-I/usr/include`
   `export WIRINGPI_LIBS=-L/usr/lib\ -lwiringPi`
8. Run
   `./configure`
9. Change into the nxprdlib directory with the command
   `cd nxprdlib`
10. Finally compile with
   `make`
11. Copy the resulting library with
   `sudo cp libnxprdlib.a /usr/local/lib/`
12. Create your own code
13. Compile your code with
   `gcc -o my-program my-code.c -Itypes -Iintfs -L/usr/lib -L/usr/local/lib -lwiringPi -lnxprdlib`
14. Run the program with
   `sudo ./my-program`
