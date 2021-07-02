#!/bin/bash

pacmd set-default-source alsa_input.usb-Sennheiser_Communications_Sennheiser_USB_headset-00.mono-fallback #set source to Sennheier
pacmd set-source-volume alsa_input.usb-Sennheiser_Communications_Sennheiser_USB_headset-00.mono-fallback 52430 #set volume to 80% 
