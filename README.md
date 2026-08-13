# Linux_Learning - USB sub driver
passed a physical USB keyboard through to a Linux VM and wrote a usb_driver that:
→ registers using vendor/product ID (found via lsusb)
→ detects connect/disconnect dynamically (hot-plug — no static Device Tree entry needed)
→ submits a URB (USB Request Block) to listen for key events
→ captures key press/release data, logged live via dmesg
→ re-submits the URB after every event, since USB completions are one-shot
→ cleans up properly with usb_kill_urb() on disconnect
