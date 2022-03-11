
//MAC_Max_STA_Num.h//

//Define to prevent recursive inclusion
#ifndef __MAC_MAX_STA_NUM_H
#define __MAC_MAX_STA_NUM_H

// распространение старшего бита вправо ( CPL2(x) )
#define OR_CPL2(x, n)			((x) | ((x) >> (n)))
#define CPL2(x)					((OR_CPL2(OR_CPL2(OR_CPL2(OR_CPL2(OR_CPL2((x) - 1, 1), 2), 4), 8), 16)) + 1)

#define MAX_RADIO_CONNECTIONS	7
#define MAX_FRAMES_PER_CYCLE	CPL2(MAX_RADIO_CONNECTIONS)

#endif // #ifndef __MAC_MAX_STA_NUM_H
