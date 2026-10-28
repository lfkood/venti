Kood kompileerumine on võimalik läbi GCC and Clangi. Ehitamiseks eemalda pico_clib muudatus CMakeLists.txt'st. Clang'i kasutuseks muuda Clangi asukoha definitsioon enda süsteemi jaoks sobilikuks.
Enne ehitamist:
- kasuta `git submodule init --recursive`
- eemalda `set(PICO_CLIB picolibc)` `CMakeLists.txt` failist

SystemView'l on typo failis "lib/SystemView/Sample/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.h" real 712.

## Audio foundation (SD card playback basis)

Repository now includes a platform-agnostic audio foundation in `/home/runner/work/venti/venti/src/audio`:
- storage abstraction (`audio_storage.h`) intended for SD-card backed file access,
- decoder abstraction (`audio_decoder.h`) with format detection for mp3/flac/ogg/wav/aac/opus/m4a,
- playback state machine (`audio_engine.*`),
- external text command parser (`audio_controller.*`) for controller-driven control flow.

For host-side simulation/testing there is an optional `venti_audio_host_sim` target (controlled by `VENTI_BUILD_AUDIO_HOST_SIM`) that can scan a local directory and exercise commands (`list`, `play`, `pause`, `stop`, `next`, `prev`, `select <n>`).