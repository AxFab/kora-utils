#!/bin/bash

set -e

TOPDIR=`pwd`
TOPDIR=`pwd`
DATE=`date '+%B %Y'`

function make_man
{
    IFS='\n';
    NAME=`basename "$1" .c`
    SRC="${1}"
    MAN="man/${NAME}.m1"
    CLI="bin/${NAME}"

    echo -e "\033[36mMake $SRC --> $CLI \033[0m";
    if [ ! -f "$SRC" ]; then
        echo -e "\033[31mMissing source file: $SRC \033[0m";
        return;
    fi
    if [ ! -f "$CLI" ]; then
        echo -e "\033[31mMissing delivery file: $CLI \033[0m";
        return;
    fi

    echo "  MAN ${NAME^}"

    FLHP=`mktemp`
    FLVS=`mktemp`
    FLAN=`mktemp`
    "$CLI" --help > "${FLHP}"
    "$CLI" --version > "${FLVS}"
    git shortlog -ns "$SRC" | cut -f 2 > "${FLAN}"


    VERS=`head -n 1 "${FLVS}" | cut -f 4 -d ' '`
    echo ".TH ${NAME^^} \"1\" \"${DATE}\" \"Kora utils ${VERS}\" \"User Commands\"" > "${MAN}"

    echo ".SH NAME" >> "${MAN}"
    MD=0
    while read -r LN; do
        if [ -z "$LN" ]; then
            MD=`expr $MD + 1`
        elif [ "$MD" == 1 ]; then
            echo "${NAME} \\- ${LN}" >> "${MAN}"
            MD=`expr $MD + 1`
        fi
    done < "${FLHP}"

    echo ".SH SYNOPSIS" >> "${MAN}"
    MD=0
    while read -r LN; do
        if [ -z "$LN" ]; then
            MD=`expr $MD + 1`
        elif [ "$MD" == 0 ]; then
            echo ".B ${NAME}" >> "${MAN}"
            LN=`echo $LN | sed "s/^.*${NAME}\s*//"`
            echo "${LN}" >> "${MAN}"
            # Parse usage line and add text decoration!
        fi
    done < "${FLHP}"

    echo ".SH DESCRIPTION" >> "${MAN}"
    MD=0
    while read -r LN; do
        if [ -z "$LN" ]; then
            MD=`expr $MD + 1`
        elif [ "$MD" == 1 ]; then
            echo ".PP" >> "${MAN}"
            echo "${LN}" >> "${MAN}"
        fi
    done < "${FLHP}"

    MD=0
    while read -r LN; do
        if [ -z "$LN" ]; then
            MD=`expr $MD + 1`
        elif [ "$MD" == 2 ]; then
            if [ "$LN" != "with options:" ]; then
                echo ".TP" >> "${MAN}"
                K=${LN:2:4}
                K=`echo "$K" | sed 's/\s*$//'`
                W=${LN:6:20}
                W=`echo "$W" | sed 's/\s*$//'`
                S=`echo "${LN:27}"`
                if [ -z "$K" ]; then
                    echo '\fB\'"$W"'\fR' >> "${MAN}"
                elif [ -z "$W" ]; then
                    echo '\fB\'"$K"'\fR' >> "${MAN}"
                else
                    echo '\fB\'"$K"'\fR, \fB\'"$W"'\fR' >> "${MAN}"
                fi
                echo "${S}" >> "${MAN}"
            fi
        fi
    done < "${FLHP}"

    # Add long description if available ...

    echo ".SH AUTHOR" >> "${MAN}"
    echo "Written by:" >> "${MAN}"
    while read -r AN; do
        echo "  ${AN}" >> "${MAN}"
    done < "${FLAN}"

    # echo ".SH \"REPORTING BUGS\"" >> "${MAN}"
    # <https://kora-os.net/reporting_bugs>

    echo ".SH COPYRIGHT" >> "${MAN}"
    echo "Copyright (C) 2015-2021  <Fabien Bavent>" >> "${MAN}"
    echo ".br" >> "${MAN}"
    echo "License AGPLv3+: GNU AGPL version 3 or later <https://www.gnu.org/licenses/agpl-3.0.html>." >> "${MAN}"
    echo ".br" >> "${MAN}"
    echo "This is free software: you are free to change and redistribute it." >> "${MAN}"
    echo "There is NO WARRANTY, to the extent permitted by law." >> "${MAN}"

    echo ".SH \"SEE ALSO\"" >> "${MAN}"
    echo "Full documentation at <https://kora-os.net/refs/${NAME}>"  >> "${MAN}"

    rm "${FLHP}"
    rm "${FLVS}"
    rm "${FLAN}"
}


if [ -z "$1" ]; then
    for FL in `ls -1 src/*.c`; do
        make_man "$FL"
    done
    for FL in `ls -1 src/zl/*.c`; do
        make_man "$FL"
    done
    for FL in `ls -1 src/ui/*.c | cat`; do
        make_man "$FL"
    done
else
    make_man "$1"
fi