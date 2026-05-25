#!/bin/bash

PROJECT="/home/mahtab/Code/tasks/sit111/arduino"

case "$1" in

pomodoro)
    echo -e "pomodoro\nstart" | python3 "$PROJECT/control.py"
    ;;

break)
    echo -e "break\nstart" | python3 "$PROJECT/control.py"
    ;;

reset)
    echo "reset" | python3 "$PROJECT/control.py"
    ;;

status)
    echo "status" | python3 "$PROJECT/control.py"
    ;;

sensor)
    echo "sensor" | python3 "$PROJECT/control.py"
    ;;

set)
    echo "set:$2" | python3 "$PROJECT/control.py"
    ;;
    
start)
    echo "start" | python3 "$PROJECT/control.py"
    ;;

stop)
    echo "stop" | python3 "$PROJECT/control.py"
    ;;

reset)
    echo "reset" | python3 "$PROJECT/control.py"
    ;;

led_on)
    echo "led:on" | python3 "$PROJECT/control.py"
    ;;

led_off)
    echo "led:off" | python3 "$PROJECT/control.py"
    ;;

watch)
    echo "watch:start" | python3 "$PROJECT/control.py"
    ;;

watchstop)
    echo "watch:stop" | python3 "$PROJECT/control.py"
    ;;

watchreset)
    echo "watch:reset" | python3 "$PROJECT/control.py"
    ;;

buzzer_on)
    echo "buzzer:on" | python3 "$PROJECT/control.py"
    ;;
    
buzzer_off)
    echo "buzzer:off" | python3 "$PROJECT/control.py"
    ;;
 

*)
    echo "Desk Station by Mahtab Hossain	Usage:"
    echo "desk pomodoro		- Start a pomodoro timer"
    echo "desk set mmss		- Set a timer with mm minutes and ss seconds"
    echo "desk start		- Start the current timer"
    echo "desk stop		- Pause the current timer"
    echo "desk reset		- Reset the current timer"
    echo "desk break		- Start a break of 5 minutes"
    echo "desk status		- Show full system status"
    echo "desk sensor		- Show sensor status"
    echo "desk led_on		- Turn LEDs on"
    echo "desk led_off		- Turn LEDs off"
    echo "desk watch		- Start the stopwatch"
    echo "desk watchstop		- Pause the stopwatch"
    echo "desk watchreset		- Reset the stopwatch"
    echo "desk buzzer:off		- Silence the buzzer"
    echo "desk buzzer:on		- Turn on the buzzer"
    ;;
esac
