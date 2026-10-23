Kood kompileerumine on võimalik läbi GCC and Clangi. Ehitamiseks eemalda pico_clib muudatus CMakeLists.txt'st. Clang'i kasutuseks muuda Clangi asukoha definitsioon enda süsteemi jaoks sobilikuks.
Enne ehitamist:
- kasuta `git submodule init --recursive`
- eemalda `set(PICO_CLIB picolibc)` `CMakeLists.txt` failist

SystemView'l on typo failis "lib/SystemView/Sample/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.h" real 712.