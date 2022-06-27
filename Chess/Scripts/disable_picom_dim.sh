#!/usr/bin/env bash

WIN_ID=$(xdotool getactivewindow)
WM_TYPE=$(xprop -notype -id $WIN_ID | grep _NET_WM_WINDOW_TYPE)
NM_TYPE="_NET_WM_WINDOW_TYPE = _NET_WM_WINDOW_TYPE_NORMAL"

#if [[ "$WM_TYPE" === "$NM_TYPE" ]]; then
xprop -id "$WIN_ID" -format _NET_WM_WINDOW_TYPE 32a -set _NET_WM_WINDOW_TYPE "_NET_WM_WINDOW_TYPE_DOCK"