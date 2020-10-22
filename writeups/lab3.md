Lab 3 Writeup
=============

My name: Ha Tran

My SUNet ID: hahntrn

This lab took me about 15.5 hours to do. I did not attend the lab session.

I worked with or talked about this assignment with: Nikhil Raguraman (nikhilvr)

Program Structure and Design of the TCPSender:
I save the outstanding segments in a map ordered by the seqno so I can 
access the earliest sent segments for resending and removing segments
that have been acknowledged.


Implementation Challenges:
Reading comprehension and following lab description. 


Remaining Bugs:
In fill window, if bytes in flight >> window size, then when calculating
number of bytes to send = window size - bytes in flight we might get an
overflow value < max payload size 

can window size > bytes in flight?

in ack received, update window size even it's less than before?

what reference is the window size?? last ackno?

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
