#include "main.h"
#include "Message.h"

// ./Message /dev/ttyS1 9600 172.16.8.23 49152

int main(int argc, char *argv[])
{
  if(argc < 5)
  {
    cout << "usage " << argv[0] << " /dev/ttyS1 9600 172.16.8.23 49152" << endl;
    exit(0);
  }
  cout << "name prog   : " << argv[0] << endl;
  cout << "serial      : " << argv[1] << endl;
  cout << "baud rate   : " << argv[2] << endl;
  cout << "udp address : " << argv[3] << endl;
  cout << "udp port    : " << argv[4] << endl;
  
  string isUdp;
  Message message;

  cout << "enter udp / uart / work (other for exit)" << endl;
  cin >> isUdp;

  if(isUdp == "udp")
  {
    message.txrxUDP(argv[3], atoi(argv[4]));
  }
  else if(isUdp == "uart")
  {
    message.txrxUART(argv[1], atoi(argv[2]));
  }
  else if(isUdp == "work")
  {
    message.workUARTUDP(argv[3], atoi(argv[4]),
                        argv[1], atoi(argv[2]));
  }

  cout << "end program !" << endl;

  return 0;
}
