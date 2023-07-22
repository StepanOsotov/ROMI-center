#ifndef MESSAGE_H
#define MESSAGE_H

#include "main.h"
#include "stdint.h"

class Message
{
  public:
    Message();

    void txrxUDP(const char *udpSocket, const uint16_t udpPort);
    void txrxUART(const char *tty, const uint32_t ttyBR);
    void workUARTUDP(const char *udpSocket, const uint16_t udpPort,
                     const char *tty, const uint32_t ttyBR);
    
    void ttyExit(int sig);

    virtual ~Message();

  protected:
  private:

    enum WHO_IS_T {IS_NONE, IS_UDP, IS_UART};

    WHO_IS_T whoIs;

    //UDP
    string mTxMessage;
    string mRxMessage;
    //const uint16_t PORT = 49152;
    const uint16_t SIZE = 1024;
    char *mBuffer;
    int mSockfd;
    int mStatus;
    struct sockaddr_in mServaddr;
    vector <string> mRxFromSerial;

    //UART
    //const char *SERIAL_DEVICE = "/dev/ttyS1";
    const int AMOUNT_TX = 10;
    FILE *mTtyRd, *mTtyWr;
    struct termio mTermSave, mStdinSave;
    int retval;
    int mSerialDevice;
    int mNum;
};

#endif // MESSAGE_H
