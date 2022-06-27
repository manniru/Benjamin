#!/usr/bin/env bash

WIN_ID=$(xdotool getactivewindow)
xprop -id "$WIN_ID" -format _NET_WM_WINDOW_TYPE 32a -set _NET_WM_WINDOW_TYPE "_NET_WM_WINDOW_TYPE_NORMAL"