#!/bin/bash


function main(){
    POSIXLY_CORRECT=yes
    check_mole_rc
    decide_what_function_to_run $@
}


function check_mole_rc(){
    if [[ -z "${MOLE_RC}" ]]; then # MOLE_RC is not set
        echo "MOLE_RC is not set" >&2
        exit 1
    else # MOLE_RC is set
        if [ ! -f "$MOLE_RC" ]; then # if MOLE_RC does not exist
            if ! (mkdir -p $(dirname "$MOLE_RC") && touch "$MOLE_RC"); then # if failed while creating MOLE_RC
                echo -e "Cannot create MOLE_RC '$MOLE_RC'" >&2
                exit 1
            fi
        fi
    fi
}


function decide_what_function_to_run(){
    # Decide between help, list and secret_log
    if [ $# -ne 0 ]; then
        if [ $1 == "-h" ]; then
            show_help
            exit 0
        elif [ $1 == "list" ]; then
            shift 1
            do_list $@
        elif [ $1 == "secret-log" ]; then
            shift 1
            do_secret_log $@
        fi
    fi

    # Decide between file and directory opening
    m=false
    g=false
    a=false
    b=false

    while getopts "mg:a:b:" arg; do
        case $arg in
            m)
                m=true
                ;;
            g)
                g=$OPTARG
                ;;
            a)
                a=$OPTARG
                ;;
            b)
                b=$OPTARG
                ;;
            *|:)
                echo Unknown option >&2
                show_help
                exit 1
                ;;
        esac
    done

    shift $(($OPTIND-1))

    if [ -z $1 ]; then # EMPTY
        do_dir $m $g $a $b $(get_dir $1)
    elif [ -f $1 ]; then # Is FILE
        if [ $m = false ] && [ $a = false ] && [ $b = false ]; then
            do_file $g $1
        else
            echo Illegal argument combination >&2
            show_help
            exit 1
        fi
    else # Is DIR
        do_dir $m $g $a $b $1
    fi
}


# Opens the $1 in the editor specified by the requirements
# usage: open_with_editor $PATH_TO_FILE
function open_with_editor(){
    if [[ -z "${EDITOR}" ]]; then # if EDITOR is not set
        if [[ -z "${VISUAL}" ]]; then # if VISUAL is not set
            vi $1
            exit $?
        else # VISUAL is set
            $VISUAL $1
            exit $?
        fi
    else # EDITOR is set
        $EDITOR $1
        exit $?
    fi
}


# Usage: echo dir: $(get_dir /home/xjanst02/)
# $1 - supposed dir
function get_dir(){
    if ! command -v realpath >/dev/null ; then
        echo realpath is not installed
        exit 1
    fi
    realpath $([ -z $1 ] && pwd || [ -d $1 ] && echo $1 || pwd)
}


function show_help(){
    echo -e "MOLE Usage:


    NOTE: Na Merlinovi z nějakého důvodu nemá můj mole oprávnění a zápis, takže jsem nemohl pořádně otestovat :(


    mole -h
        Shows this help

    mole [-g GROUP] FILE
        Opens the FILE. If -g is used, the FILE be added to the GROUP

    mole [-m] [FILTERS] [DIRECTORY]
        Opens last opened file in DIRECTORY.
        If -m is used, opens the most edited file in DIRECTORY.
        If no DIRECTORY is entered, current directory is used
        FILTERS can be used

    mole list [FILTERS] [DIRECTORY]
        Lists all the files and their groups.
        If DIRECTORY is used, only files in that DIRECTORY will be shown
        FILTERS can be used
    
    FILTERS
        [-g GROUP]
            Limits the output to files of given GROUP
        [-a DATE]
            Limits the output to files that have been editer after DATE
        [-b DATE]
            Limits the output to files that have been editer before DATE
"
}


# $1 g ; $2 FILE
function do_file(){
    file_groups=$( tac $MOLE_RC | grep -m 1 $2 | cut -d " " -f 3- )
    if !(grep -q $1 <<< $file_groups) ;then # If not already in group $1
        if [ $1 != false ]; then # if group $1 is not epmty
            [ -z $file_groups ] && file_groups=$1 || file_groups="$file_groups $1"
        fi
    fi
    # Log it
    if ! command -v realpath >/dev/null ; then
        echo realpath is not installed
        exit 1
    fi
    echo $(date +%s)" "$(realpath $2)" "$file_groups >> $MOLE_RC
    open_with_editor $2
    exit 0
}


# $1 m ; $2 g ; $3 a ; $4 b ; $5 DIR
function do_dir(){
    [ $3 != false ] && after=$(date -d ${3} +%s)
    [ $4 != false ] && before=$(date -d ${4} +%s)
    # Match DIRs
    # filter_matching_lines=$(awk -v dir="$5" '{if (index($2, dir) == 1) printf "%s\n", $0}' $MOLE_RC)
    filter_matching_lines=$(awk -v dir="$5" '$2 ~ "^"dir"/[^/]+$"' $MOLE_RC)
    # March groups
    [ $2 != false ] && filter_matching_lines=$(awk -v groups="$2" '{split(groups,arr,","); for(i=3;i<=NF;i++) for(j in arr) if($i==arr[j]) {print ; break}}' <<< $filter_matching_lines)
    # Match after
    [ ! -z $after ] && filter_matching_lines=$(awk -v after="$after" '$1 > after {print}' <<< $filter_matching_lines)
    # Match before
    [ ! -z $before ] && filter_matching_lines=$(awk -v before="$before" '$1 < before {print}' <<< $filter_matching_lines)
    # Exit if no filter maches have been found
    [[ -z $filter_matching_lines ]] && echo No matching files found && exit 1

    if [ $1 == true ]; then # Open most edited
        file=$(awk '{print $2}' <<< $filter_matching_lines | sort | uniq -c | sort -nr | head -n1 | awk '{print $2}')
    else # Open last edited
        file=$( tail -n 1 <<< $filter_matching_lines | cut -d " " -f 2 )
    fi

    # Open the desired file
    do_file false $file

    exit 0
}


function do_list(){
    while getopts "g:a:b:" arg; do
        case $arg in
            g)
                g=$OPTARG
                ;;
            a)
                after=$(date -d ${OPTARG} +%s) || exit 1
                ;;
            b)
                before=$(date -d ${OPTARG} +%s) || exit 1
                ;;
            *|:)
                echo Unknown or invalid option >&2
                show_help
                exit 1
                ;;
        esac
    done

    shift $(($OPTIND-1))
    dir=$(get_dir $1)
    # Match DIRs
    # filter_matching_lines=$(awk -v dir="$dir" '{if (index($2, dir) == 1) printf "%s\n", $0}' $MOLE_RC)
    filter_matching_lines=$(awk -v dir="$dir" '$2 ~ "^"dir"/[^/]*$"' "$MOLE_RC")
    # Match groups
    [ ! -z $g ] && filter_matching_lines=$(awk -v groups="$g" '{split(groups,arr,","); for(i=3;i<=NF;i++){ for(j in arr){ if($i==arr[j]) {print}}}}' <<< $filter_matching_lines)
    # Match after
    [ ! -z $after ] && filter_matching_lines=$(awk -v after="$after" '$1 > after {print}' <<< $filter_matching_lines)
    # Match before
    [ ! -z $before ] && filter_matching_lines=$(awk -v before="$before" '$1 < before {print}' <<< $filter_matching_lines)
    # Exit if no filter maches have been found
    [[ -z $filter_matching_lines ]] && exit 1
    # Get only the last logs for each file
    filter_matching_lines="$(awk '{a[$2]=$0} END{for(i in a) print a[i]}' <<< $filter_matching_lines | cut -d " " -f 2-)"


    # Get basenames
    names=$(cut -d " " -f 1 <<< $filter_matching_lines | awk -F/ '{print $NF}')

    # Get groups
    groups=$(awk '{ if (NF == 1) { print "-" } else { for (i=2; i<=NF; i++) { printf("%s ", $i) } printf("\n") } }' <<< $filter_matching_lines)
    # Sort the groups
    groups=$(awk '{ split($0, groups, " "); asort(groups); sorted_line = groups[1]; for (i = 2; i <= length(groups); i++) { sorted_line = sorted_line " " groups[i] } print sorted_line }' <<< $groups)
    # Replace spaces with commans in groups
    groups=$(tr " " "," <<< $groups)
    
    # Combine the names and groups together with indents
    list=$(paste <(echo "$names") <(echo "$groups") | awk '{ max = length($1) > max ? length($1) : max } { a[NR]=$1 ":"; b[NR]=$2 } END { for (i=1; i<=NR; i++) printf("%-*s %s\n", max+1, a[i], b[i]) }')

    # Sort the lines based on the names
    echo "$(sort -t: -k1,1 <<< $list)"
    exit 0
}


function do_secret_log(){
    while getopts "a:b:" arg; do
        case $arg in
            a)
                after=$(date -d ${OPTARG} +%s) || exit 1
                ;;
            b)
                before=$(date -d ${OPTARG} +%s) || exit 1
                ;;
            *|:)
                echo Unknown or invalid option >&2
                show_help
                exit 1
                ;;
        esac
    done

    shift $(($OPTIND-1))

    # Match DIRs
    filter_matching_lines=""
    if [ $# -ne 0 ]; then
        if ! command -v realpath >/dev/null ; then
            echo realpath is not installed
            exit 1
        fi
        for dir in "$@"; do
            dir=$(realpath $dir)
            filter_matching_lines+=$(awk -v dir_path="$dir" '{if (index($2, dir_path) == 1) printf "%s\n", $0}' $MOLE_RC)"\n" 
        done
    else
        filter_matching_lines=$(cat $MOLE_RC)
    fi
    
    
    # No group matching in specification :(


    # Match after
    [ ! -z $after ] && filter_matching_lines=$(awk -v after="$after" '$1 > after {print}' <<< $filter_matching_lines)
    # Match before
    [ ! -z $before ] && filter_matching_lines=$(awk -v before="$before" '$1 < before {print}' <<< $filter_matching_lines)

    # Construct the log (for each file have multiple timestamps)
    log="$(awk '{printf("%s %s\n",$2,strftime("%Y-%m-%d_%H-%M-%S", $1))}' <<< $filter_matching_lines | awk '{a[$1]=a[$1]";"$2;}END{for (i in a) print i""a[i]}' | sort -t ';' -k 1)"
    log_path=/home/${USER}/.mole/log_${USER}_$(date +%Y-%m-%d_%H-%M-%S).bz2
    echo $log_path
    # Create the log file    
    if ! (mkdir -p $(dirname "$log_path") && touch "$log_path"); then # if failed while creating log
        echo -e "Cannot create log '$log_path'" >&2
        exit 1
    fi
    # Write and compress the log
    echo "$log" | bzip2 > $log_path

    exit 0
}


main $@