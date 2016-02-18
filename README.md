## SREEB - Simple Research Equipment Extension Box

Control external scientific equippment using the Arduino-based Robot Controller Shield by Watterott 
(https://github.com/watterott/Robot-Controller/).

The software sets up a simple communication protocol that can be extended by the user (see libraries/RMsg). 
The client sends commands or requests as ASCII strings via the serial/USB connection to the Arduino controller 
(the host), which acknowledges the command or replies by sending data.

Commands or requests always have the form:

``>CMD A=d1,d2,d3... B=d4,d5,...;``
  
with ``>`` indicating the start and ``;`` the end of a command, ``CMD`` the command token, ``A``, ``B``, ... optional
parameters with ``d1``, ``d2``, ... numerical data values (16-bit integers).

The reply from the controller (host) starts with a ``<`` and ends with ``;`` like client commands/requests. 
The controller can reply to three ways: by acknowledging the command/request (``ACK``), by returning an error
(``ERR``), or by returning data (in this case the returned token is the same as the command token, see e.g.
``VER`` below).

- Returning an error code regarding the last message

  ``<ERR C=x E=y,z;``

  with ``x``, command index (255=not recognized), ``y``, error code, and ``z``, error value (further specifies
  the error). 

- Acknowledging that command has been executed; when no error occurred and the command has no specific response
  message defined (other than e.g. ``>VER;``)
  
  ``<ACK C=x;``
  
  with ``x``, command index
  
####Currently available commands:

- Information about software version (V) and free space in SRAM (M) in bytes

  ``>VER;``

  Returns something like:
  
  ``<VER V=100 M=1234;``
  
- Define a servo port, two servo positions and a port that serves as input to toggle between these 
  two servo positions.
  
  ``>SDT P=s,i,o S=a,b;``
  
	with ``s``, servo port index (1...8) as output to servo, ``i``, servo port index (1...8) as input to 
	toggle servo, ``o``, servo port index (1...8) as output to indicate servo position "a", ``a`` and ``b``, 
	two servo positions (0...255) for the signal at ``i`` being LOW or HIGH, respectively.


- Define I/O mode of up to 8 digital pins (=servo ports). 

  ``>SDM P=x1,x2... M=y1,y2...;``
  
  with ``x1,..``, servo port index (1...8), ``y1,..`` modes (0=input, 1=input_low, 2=output, 3=servo). Note 
  that mode == 0 (input) requires an external pulldown resistor, whereas mode == 1 (iput low) uses the
  internal 2k pullup resistor.
  
- Set up to 8 digital pin (=servo ports) values simultanously.
  
  ``>SDV P=x1,x2... V=y1,y2...;``
  
  with ``x1,..`` servo port index (1...8), ``y1,..``values. For output pins, 0=low, 1=high, and for 
  servo pins, 0..255 as angular position.  
  
- Write data via I2C protocol.

   ``>I2W A=x, B=y, W=z;``
   
   with ``x`` being the address to be written to, ``y`` the value to be written and
   ``z`` the delay after writingm the value.

- Read data via I2C protocol.

   ``>I2R A=x, B=y, W=z;``
   
   with ``x`` being the address to be written to, ``y`` the number of bytes to be read
   and ``z`` the delay after reading a byte.

- Clear all settings.
 
  ``>CLR;``
