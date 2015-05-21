#ifndef SPLASHMARKER_H
#define SPLASHMARKER_H

// this string is used to find the splash screen bitmap.
// the bitmap should follow this string and be 128x64 bytes long.
// this is done so eePe can find the splash screen and replace it if need be.

const prog_uchar APM s9xsplashMarker[] = {
"Splash"
};

#endif // SPLASHMARKER_H
