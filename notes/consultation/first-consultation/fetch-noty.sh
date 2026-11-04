#!/usr/bin/env bash
# Fill noty/ with the datasheet (nota katalogowa) of every part on the prototype and the target design.
# Promotor rule (teams-dump.md, "Jak przygotować się do konsultacji"): bring "noty techniczne (pdf)
# do stosowanych elementów". noty/ is git-ignored (third-party PDFs); this list is the record.
# Run from the repo root: notes/consultation/first-consultation/fetch-noty.sh
# Needs wget and chromium (DCF manual is only an HTML dump). Existing files are kept.
set -uo pipefail
dir=notes/consultation/first-consultation/noty
mkdir -p "$dir"
fail=0

get() { # get <file name> <local path or URL>
  local out="$dir/$1" src=$2
  [[ -s $out ]] && return
  if [[ $src == http* ]]; then
    wget -q -U 'Mozilla/5.0' -O "$out" "$src" || { rm -f "$out"; echo "FAIL $1 <- $src"; fail=1; return; }
  else
    cp "$src" "$out" 2>/dev/null || { echo "FAIL $1 <- $src (local file missing)"; fail=1; return; }
  fi
  head -c 5 "$out" | grep -q '%PDF' || { rm -f "$out"; echo "FAIL $1 <- $src (not a PDF)"; fail=1; }
}

# Prototype on the desk
get 01_nRF52832_Product_Specification_v1.9.pdf temp-resources/docs/nrf/nRF52832_PS_v1.9.pdf
get 02_nRF52_DK_User_Guide_v3.pdf               temp-resources/docs/nrf/nRF52_DK_User_Guide_v3.x.x.pdf
get 03_nRF52_DK_PCA10040_schemat.pdf            "temp-resources/nRF52 Development Kit - Hardware files 3_0_0/PCA10040-nRF52832 Development Board 3_0_0/Schematic_Layout pdf files/PCA10040_Schematic_And_PCB.pdf"
get 04_Waveshare_2.13_e-Paper_specyfikacja.pdf  temp-resources/2.13inch_e-Paper_Specification.pdf
get 05_Waveshare_2.13_e-Paper_HAT_schemat.pdf   https://files.waveshare.com/upload/5/57/2.13inch_e-Paper_HAT_Schematic.pdf
get 06_SSD1680_sterownik_e-Paper.pdf            https://cdn-learn.adafruit.com/assets/assets/000/097/631/original/SSD1680_Datasheet.pdf
get 07_LIS3DH.pdf                               https://www.st.com/resource/en/datasheet/lis3dh.pdf
get 08_Adafruit_LIS3DH_breakout.pdf             https://cdn-learn.adafruit.com/downloads/pdf/adafruit-lis3dh-triple-axis-accelerometer-breakout.pdf
get 09_BSS138_na_plytce_LIS3DH.pdf              https://www.onsemi.com/pdf/datasheet/bss138-d.pdf
get 11_MAS6180B_odbiornik_w_DCF-1060N.pdf       https://www.mas-oy.com/wp-content/uploads/2016/04/DA6180B_005.pdf
get 12_2N3904.pdf                               temp-resources/2N3904_datasheet.pdf
get 13_1N4148.pdf                               https://www.vishay.com/docs/81857/1n4148.pdf
# AP-1205V-P1 has no datasheet online (searched 2026-11-04); HCM12 is the 5 V magnetic buzzer
# the breadboard guide uses as its reference.
get 14_brzeczyk_HCM12_zamiennik_AP-1205V-P1.pdf temp-resources/HCM12.pdf
# Target design (block diagram) and battery path
get 15_MDBT42Q-512KV2.pdf                       https://www.raytac.com/upload/download_files/38a8a4a0aff945d8484507d60058109b.pdf
get 16_MCP73831_ladowarka.pdf                   https://ww1.microchip.com/downloads/en/DeviceDoc/20001984g.pdf
get 17_AP2112_LDO.pdf                           https://www.diodes.com/assets/Datasheets/AP2112.pdf
get 18_Pimoroni_LiPo_Amigo_schemat.pdf          temp-resources/lipo_amigo_schematic.pdf

# DCF-1060N-800: vendor manual exists only as the saved web page
dcf=$dir/10_DCF-1060N-800_instrukcja.pdf
if [[ ! -s $dcf ]]; then
  chromium --headless --disable-gpu --no-pdf-header-footer --print-to-pdf="$dcf" \
    "file://$PWD/temp-resources/docs/dcf77/dcf77-manual-dump.html" 2>/dev/null \
    || { echo "FAIL 10_DCF-1060N-800_instrukcja.pdf (chromium)"; fail=1; }
fi

ls -1 "$dir"
exit $fail
