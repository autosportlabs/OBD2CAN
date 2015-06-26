#!/bin/bash

## exports PNG-format images of each sheet of each schematic, and each board
## only works with XML-format .sch files

set -e -u

_EAGLE="$HOME/Applications/EAGLE/EAGLE.app/Contents/MacOS/EAGLE"
board_dir="$( cd $(dirname $0) && /bin/pwd )"

cd "${board_dir}"

tmpf=$( mktemp -t exportXXXX )
mv ${tmpf} ${tmpf}.scr
tmpf="${tmpf}.scr"

for sch in *.sch; do
    sch_base="${sch%.sch}"
    
    rm -f ${sch_base}_s?.sch.png ${sch_base}.parts.txt
        
    echo "edit ${sch};" >> ${tmpf}
    echo "export partlist '${sch_base}.parts.txt';" >> ${tmpf}
    
    num_sheets=$( fgrep '<sheet>' ${sch} | wc -l )
    for sheet_num in $( seq 1 ${num_sheets} ); do
        echo "edit .s${sheet_num};" >> ${tmpf}
        echo "export image '${sch_base}_s${sheet_num}.sch.png' 300;" >> ${tmpf}
    done
done

_first_board=""
for brd in *.brd; do
    if [ -z "${_first_board}" ]; then
        _first_board=$brd
    fi
    
    rm -f ${brd}.png
    
    echo "edit ${brd};" >> ${tmpf}
    echo "ratsnest;" >> ${tmpf}
    echo "export image '${brd}.png' 300;" >> ${tmpf}
done

## avoid prompt to save the board file; Eagle thinks it's changed, but it
## hasn't.
echo "write;" >> ${tmpf}
echo "quit;" >> ${tmpf}

"${_EAGLE}" -C "script ${tmpf}" ${_first_board}

## parts list gets exported as iso-8859-1
for x in *.parts.txt; do
   iconv -f iso-8859-1 -t utf-8 ${x} | expand | egrep -v '^Exported from ' > ${x}.converted
   mv ${x}.converted ${x}
done

rm -f ${tmpf}
