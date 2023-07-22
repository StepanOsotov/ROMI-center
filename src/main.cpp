#include "main.h"
#include "Message.h"

int main()
{
  string isUdp;
  Message message;

  cout << "enter udp or uart (other for exit)" << endl;
  cin >> isUdp;

  if(isUdp == "udp")
  {
    message.txrxUDP();
  }
  else if(isUdp == "uart")
  {
    message.txrxUART();
  }

  cout << "end program !" << endl;

  return 0;
}
