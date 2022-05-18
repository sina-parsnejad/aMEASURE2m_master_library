/* Microduino-CoreUSB
 * UART library
 * Created by Gtat, 2016
 * Email gtatyous@msu.edu
*/ 

void USART_Init(unsigned int baud)
{
  /*Set baud rate */
  UBRRHn = (unsigned char) (baud >> 8 );
  UBRRLn = (unsigned char) baud;

  /* Enable receiver and transmitter */
  UCSRnB = (1<<RXENn) | (1<<TXENn);

  /* Set frame format: 8data, 2stop bit */
  UCSRnC = (1<<USBSn) | (3<<UCSZn0);
}

