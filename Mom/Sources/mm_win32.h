#ifndef MM_WIN32_H
#define MM_WIN32_H

#define MM_DELAY_CLICK 200
#define MM_MOUSE_LKEY 1
#define MM_MOUSE_MKEY 2
#define MM_MOUSE_RKEY 3

void mm_winSleep();
void mm_sendMouseKey(int key);
void mm_sendMouseFlag(int flag);

#endif // MM_WIN32_H
