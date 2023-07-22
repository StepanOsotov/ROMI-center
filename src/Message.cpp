#include "Message.h"

//==============================================================================

Message::Message()
{
  whoIs = IS_NONE;
  mBuffer = new char [SIZE];
  cout << "Message() Constructor" << endl;
}

//==============================================================================

void Message::txrxUDP(void)
{
  whoIs = IS_UDP;
  cout << "switch UDP Protocol" << endl;
  cout << "udp server 172.16.8.23:49152" << endl;

  // Creating socket file descriptor
/*
-------------------
int sockID = socket(family, type, protocol);
-------------------
family: It is an integer in communication domain.
There are 2 possible domains:
1. Unix Domain: Where 2 process share the same file system. In that case family will be 'AF_UNIX'
2. Internet Domain: They are 2 hosts on the Internet.In that case family will be 'AF_INET'
-------------------
type: It is the type of communication.
SOCK_STREAM: For TCP
SOCK_DGRAM: For UDP
-------------------
protocol: It will specify the protocol.
IPPROTO_TCP, IPPROTO_UDP.
But it is always set to 0, so that OS will choose appropriate protocol.
-------------------
This API will return -1 upon failure.
-------------------
*/

  mSockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if ( mSockfd < 0 )
  {
    perror("socket creation failed");
    //exit(EXIT_FAILURE);
    return;
  }

  memset(&mServaddr, 0, sizeof(mServaddr));

  // Filling server information
  mServaddr.sin_family = AF_INET;
  mServaddr.sin_port = htons(PORT);
  mServaddr.sin_addr.s_addr = inet_addr("172.16.8.23");

  int n;
  socklen_t len;

  while(1)
  {
    cout << "enter message for sent.(other exit)" << endl;
    cin >> mTxMessage;

    if(mTxMessage == "exit")
    {
      cout << "exit from programm" << endl;

      sendto(mSockfd, (const char *)mTxMessage.c_str(), mTxMessage.length(),
        MSG_CONFIRM, (const struct sockaddr *) &mServaddr,
          sizeof(mServaddr));

      break;
    }

    sendto(mSockfd, (const char *)mTxMessage.c_str(), mTxMessage.length(),
      MSG_CONFIRM, (const struct sockaddr *) &mServaddr,
        sizeof(mServaddr));

    cout << "message tx : " << mTxMessage << endl;

    unsigned long valTimeout = 6000;
    //без этого вызова висим вечно
    setsockopt (mSockfd,
                SOL_SOCKET,
                SO_RCVTIMEO,
                (const char*)&valTimeout,
                sizeof(unsigned long));

    n = recvfrom(mSockfd, (char *)mBuffer, SIZE,
          MSG_WAITALL, (struct sockaddr *) &mServaddr,
          &len);
    mBuffer[n] = '\0';

    cout << "from Server rx:" << mBuffer << endl;

  }

  mStatus = close(mSockfd);

	cout << "status close() : " << mStatus << endl;

}

//==============================================================================
/*
static void Message::ExitFromProg(int sig)
{
  exit(sig);
}
*/
//==============================================================================

void Message::ttyExit(int sig)
{

  fprintf(stdout, "press exit, sig = %d\n", sig);
  if (mTtyRd)
  {
    fclose(mTtyRd);
  }
  if (mTtyWr)
  {
    fclose(mTtyWr);
  }

  ioctl(mSerialDevice, TCSETA, &mTermSave);
  close(mSerialDevice);
  ioctl(fileno(stdin), TCSETA, &mStdinSave);

  exit(sig);
//  Message::ExitFromProg(sig);
}

void Message::txrxUART(void)
{

  whoIs = IS_UART;
  cout << "switch UART Protocol" << endl;
  cout << "device /dev/ttyS1, Baud Rate = 9600" << endl;
  struct termio term, tstdin;
  int baud = B9600;
  int tx_counter = AMOUNT_TX;
  size_t sizeTx;
  mSerialDevice = open(SERIAL_DEVICE, O_RDWR | O_NDELAY);
  if (mSerialDevice < 0)
  {
    fprintf(stdout, "Failed to open SERIAL_DEVICE");
    //exit(1);
    return;
  }
  else
  {
    fprintf(stdout, "success open %s (%d)\n", SERIAL_DEVICE, mSerialDevice);
  }

  //-----------------WRITE-READ-BEGIN----------------------

  //
  ioctl(mSerialDevice, TCGETA, &mTermSave);
  ioctl(fileno(stdin), TCGETA, &mStdinSave);

//  signal(SIGHUP, ttyExit);
//  signal(SIGINT, ttyExit);
//  signal(SIGQUIT, ttyExit);
//  signal(SIGTERM, ttyExit);
  //
  ioctl(fileno(stdin), TCGETA, &tstdin);
  tstdin.c_iflag = 0;
  tstdin.c_lflag &= ~(ICANON | ECHO);
  tstdin.c_cc[VMIN] = 0;
  tstdin.c_cc[VTIME] = 0;
  ioctl(fileno(stdin), TCSETA, &tstdin);
  //
  ioctl(mSerialDevice, TCGETA, &term);
  term.c_cflag |= CLOCAL|HUPCL;
  if (baud > 0)
  {
    term.c_cflag &= ~CBAUD;
    term.c_cflag |= baud;
  }
  term.c_lflag &= ~(ICANON | ECHO); //
  term.c_iflag &= ~ICRNL; //
  term.c_cc[VMIN] = 0;
  term.c_cc[VTIME] = 10;
  ioctl(mSerialDevice, TCSETA, &term);
  fcntl(mSerialDevice, F_SETFL, fcntl(mSerialDevice, F_GETFL, 0) & ~O_NDELAY);
  //
  if ((mTtyRd = fopen(SERIAL_DEVICE, "r")) == NULL )
  {
    perror(SERIAL_DEVICE);
    exit(errno);
  }
  if ((mTtyWr = fopen(SERIAL_DEVICE, "w")) == NULL )
  {
    perror(SERIAL_DEVICE);
    exit(errno);
  }
  fprintf(stdout, "success open mTtyWr = 0x%p, mTtyRd = 0x%p,\n", mTtyWr, mTtyRd);
  //

  while(tx_counter--)
  {
    snprintf(mBuffer, SIZE, "Hello Stepan, cnt %d\n", tx_counter);
    sizeTx = strlen(mBuffer);
    write(fileno(stdout), mBuffer, sizeTx);
    write(fileno(mTtyWr),    mBuffer, sizeTx);
    //fwrite("Hello\n", 6, 6, stdout);
    usleep(100000);
//    USART3_SendByte('D');
//    sizeTx += fwrite("Hello\n", 6, 6, mTtyWr);
//    printf("tx data len = %d (%d \\ %d)\n", sizeTx, tx_counter, AMOUNT_TX);
//    usleep(500000);
  }

  fprintf(stdout, "work echo\n");
  fprintf(stdout, "enter exit for quit\n");

  int rdChar;
  int buffCount = 0;

  while(1)
  {
    if ((mNum = read(fileno(stdin), &rdChar, 1)) > 0)
    {
      mBuffer[buffCount] = static_cast<char>(rdChar);
      buffCount++;
      cout << static_cast<char>(rdChar) << endl;

      if(static_cast<char>(rdChar) == 0x0D)
      {
        //cout << "count = " << buffCount << endl;
        write(fileno(mTtyWr), &mBuffer[0], buffCount);
        buffCount = 0;

        if(!memcmp(&mBuffer[0], "exit", 4))
        {
          break;
          //ttyExit(0);
        }
      }
    }
/*
    if ((mNum = read(fileno(stdin), mBuffer, SIZE)) > 0)
    {
      if(mNum == 0x0D)  // press Enter
      {
        if(!memcmp(mBuffer, "exit", 4))
        {
          ttyExit(0);
        }
        write(fileno(mTtyWr), mBuffer, mNum);
      }
    }
*/
    if ((mNum = read(fileno(mTtyRd), mBuffer, SIZE)) > 0)
    {
      write(fileno(stdout), mBuffer, mNum);
//      snprintf(mBuffer, SIZE, "num byte rx %d\n", mNum);
//      write(fileno(stdout), mBuffer, mNum);
    }
  }
}

//==============================================================================

Message::~Message()
{
  delete [] mBuffer;
  cout << "Message() Destructor" << endl;

  if(whoIs == IS_UART)
  {
    ttyExit(0);
  }
}

//==============================================================================
