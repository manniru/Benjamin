#ifndef CH_KEYBOARD_W_H
#define CH_KEYBOARD_W_H

#include <QString>

void ch_returnFocus();
void ch_sendKey(int key_val);
void ch_pressKey(int key_val);
void ch_releaseKey(int key_val);
QString ch_getActivePName();

#endif // CH_KEYBOARD_W_H
