#include "Message.h"

//==============================================================================

Message::Message()
{
  whoIs = IS_NONE;
  mBuffer = new char [SIZE];
  cout << "Message() Constructor" << endl;
  mRxFromSerial.clear();
}

//==============================================================================

void Message::txrxUDP(const char *udpSocket, const uint16_t udpPort)
{
  whoIs = IS_UDP;
  cout << "switch UDP Protocol" << endl;
  cout << "udp server " << udpSocket << " : "
       << udpPort << endl;

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
  mServaddr.sin_addr.s_addr = inet_addr(udpSocket);
  mServaddr.sin_port = htons(udpPort);

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

void Message::encrypt(vector <string> *msgRx, vector <string> *msgEncrypt)
{
  string lString;
  uint16_t length;
  msgEncrypt->clear();
	
	char *lchar = new char [SIZE];

  cout << "encrypt begin >>>>>>>>>>>" << endl;

  for(vector<string>::iterator it = msgRx->begin();
      it != msgRx->end(); it++)
  {
    lString = *it;
    cout << lString.c_str();

    length = lString.length();

    strcpy(lchar, lString.c_str());

    for(uint16_t i = 0; i < length-2; i++)
    {
      (*(lchar + i))++;
    }

    //cout << lchar << endl;

    msgEncrypt->push_back(lchar);

  }

  cout << "==================" << endl;

  for(vector<string>::iterator it = msgEncrypt->begin();
      it != msgEncrypt->end(); it++)
  {
    lString = *it;
    cout << lString.c_str();
  }

  cout << "encrypt end >>>>>>>>>>>" << endl;
	
	delete [] lchar;
}

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
}

//==============================================================================

void Message::txrxUART(const char *tty, const uint32_t ttyBR)
{
  whoIs = IS_UART;
  cout << "switch UART Protocol" << endl;
  cout << "device " << tty << ", Baud Rate = " << ttyBR << endl;
  struct termio term, tstdin;
  int tx_counter = AMOUNT_TX;
  size_t sizeTx;
  mSerialDevice = open(tty, O_RDWR | O_NDELAY);
  if (mSerialDevice < 0)
  {
    cout << "Failed to open  : " << tty << endl;
    //exit(1);
    return;
  }
  else
  {
    fprintf(stdout, "success open %s (%d)\n", tty, mSerialDevice);
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
  if (ttyBR > 0)
  {
    term.c_cflag &= ~CBAUD;
    term.c_cflag |= ttyBR;
  }
  term.c_lflag &= ~(ICANON | ECHO); //
  term.c_iflag &= ~ICRNL; //
  term.c_cc[VMIN] = 0;
  term.c_cc[VTIME] = 10;
  ioctl(mSerialDevice, TCSETA, &term);
  fcntl(mSerialDevice, F_SETFL, fcntl(mSerialDevice, F_GETFL, 0) & ~O_NDELAY);
  //
  if ((mTtyRd = fopen(tty, "r")) == NULL )
  {
    perror(tty);
    exit(errno);
  }
  if ((mTtyWr = fopen(tty, "w")) == NULL )
  {
    perror(tty);
    exit(errno);
  }
  fprintf(stdout, "success open mTtyWr = 0x%p, mTtyRd = 0x%p,\n", mTtyWr, mTtyRd);
  //
  
  while(tx_counter--)
  {
    snprintf(mBuffer, SIZE, "Hello Stepan, cnt %d\n", tx_counter);
    sizeTx = strlen(mBuffer);
    write(fileno(stdout), mBuffer, sizeTx);
    write(fileno(mTtyWr), mBuffer, sizeTx);
    //fwrite("Hello\n", 6, 6, stdout);
    usleep(100000);
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
    if ((mNum = read(fileno(mTtyRd), mBuffer, SIZE)) > 0)
    {
      write(fileno(stdout), mBuffer, mNum);
//      snprintf(mBuffer, SIZE, "num byte rx %d\n", mNum);
//      write(fileno(stdout), mBuffer, mNum);
    }
  }
}

//==============================================================================

void Message::workUARTUDP(const char *udpSocket, const uint16_t udpPort,
                          const char *tty, const uint32_t ttyBR)
{
  //-------------------------------------------------------------------
  //++++++++++++++++----Open-Descripter----
  cout << "UART Protocol" << endl;
  cout << "device " << tty << ", Baud Rate = " << ttyBR << endl;
  mSerialDevice = open(tty, O_RDWR | O_NDELAY);
  if (mSerialDevice < 0)
  {
    cout << "Failed to open  : " << tty << endl;
    return;
  }
  else
  {
    cout << "success open " << tty << " (" << mSerialDevice << ")" << endl;
  }
  //++++++++++++++++----read-and-rewrite-parameter----
  struct termio term;
  ioctl(mSerialDevice, TCGETA, &term);
  term.c_cflag |= CLOCAL|HUPCL;
  if (ttyBR > 0)
  {
    term.c_cflag &= ~CBAUD;
    term.c_cflag |= ttyBR;
  }
  term.c_lflag &= ~(ICANON | ECHO); //
  term.c_iflag &= ~ICRNL; //
  term.c_cc[VMIN] = 0;
  term.c_cc[VTIME] = 10;
  ioctl(mSerialDevice, TCSETA, &term);
  //++++++++++++++++----open-to-read-data
  if ((mTtyRd = fopen(tty, "r")) == NULL )
  {
    perror(tty);
    exit(errno);
  }
  cout << "success open mTtyRd = 0x" << &mTtyRd << endl;
  cout << "wait 10 message, or limit 5 sec" << endl;

  //++++++++++++++++----

  uint16_t amountRd = 10;
  int rdChar;
  int rdCount = 0;
  time_t beginTime = time(nullptr);
  time_t nowTime;
  uint32_t deltaMs = 0;
  uint32_t changeTime = 0;

  while(1)
  {
    //++++++++++++++++----
    nowTime = time(nullptr);
    deltaMs = nowTime - beginTime;
    if( deltaMs > 5 )
      break;
    if(deltaMs != changeTime)
    {
      cout << "time : " << deltaMs << endl;
      changeTime = deltaMs;
    }
    //++++++++++++++++----
    if ((mNum = read(fileno(mTtyRd), &rdChar, 1)) > 0)
    {
      mBuffer[rdCount] = static_cast<char>(rdChar);
      rdCount++;
      //cout << static_cast<char>(rdChar) << endl;

      //wait Enter
      if(static_cast<char>(rdChar) == 0x0D)
      {
        cout << "read (" << rdCount << ") : " << mBuffer << endl;

        mRxFromSerial.push_back(mBuffer);
        beginTime = time(nullptr);
        changeTime = 0;
        
        memset(mBuffer, 0, rdCount);
        rdCount = 0;
        nowTime = time(nullptr);
        cout << "time : " << nowTime;
				amountRd--;
				cout << ", amount from tty : " << amountRd << endl;
        if(!amountRd)
        {
          break;
        }
      }
    }
    //++++++++++++++++----
  }

  mStatus = close(mSerialDevice);

  cout << "status close() : " << mStatus << endl;

  //-------------------------------------------------------------------

  cout << "UDP Protocol" << endl;
  cout << "udp server " << udpSocket << " : "
       << udpPort << endl;

  mSockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if ( mSockfd < 0 )
  {
    perror("socket creation failed");
    //exit(EXIT_FAILURE);
    return;
  }
  else
  {
    cout << "success open " << udpSocket << endl; 
  }

  memset(&mServaddr, 0, sizeof(mServaddr));

  // Filling server information
  mServaddr.sin_family = AF_INET;
  mServaddr.sin_addr.s_addr = inet_addr(udpSocket);
  mServaddr.sin_port = htons(udpPort);

  encrypt(&mRxFromSerial, &mTxToUdp);

  string strGet;
  int n;
  socklen_t len;

  cout << " vector" << endl;
  for(vector<string>::iterator it = mTxToUdp.begin();
      it != mTxToUdp.end(); it++)
  {
    strGet = *it;
    //++++++++++++++++----
    sendto(mSockfd, (const char *)strGet.c_str(), strGet.length(),
      MSG_CONFIRM, (const struct sockaddr *) &mServaddr,
        sizeof(mServaddr));

    cout << "message tx : " << strGet;

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
    
    this_thread::sleep_for(chrono::milliseconds(500) );

    cout << "from Server rx:" << mBuffer << endl;
    //++++++++++++++++----
    //cout << *it;
  }
  cout << endl;
  
  mTxMessage = "exit";

  sendto(mSockfd, (const char *)mTxMessage.c_str(), mTxMessage.length(),
    MSG_CONFIRM, (const struct sockaddr *) &mServaddr,
      sizeof(mServaddr));

  mStatus = close(mSockfd);

  cout << "status close() : " << mStatus << endl;
  //-------------------------------------------------------------------
}

//==============================================================================

Message::~Message()
{
  cout << "Message() Destructor" << endl;
  delete [] mBuffer;

  if(whoIs == IS_UART)
  {
    ttyExit(0);
  }
}

//==============================================================================
