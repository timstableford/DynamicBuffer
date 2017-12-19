#Fixed Fragmented Buffer Allocation Utility

Allocation system to allow fragmented memory buffers in a fixed size suepr-buffer.


This class is mostly useful on memory constrained devices such as the Arduino.
On those devices you may want to create and delete arrays of objects reasonably often.
Unless very careful that causes memory fragmentation that leads to an eventual crash.


This buffering system allocates a fixed array when its created and splits that array into slots.
Additionally even if allocated slots are non-contiguous they can still be accessed as if they are.


For example in a 3 slot buffer [1, 2, 3] if slot 2 is taken and the user requests 2 slots of storage
then they're assigned slots 1 and 3. They can then get a class from the DynamicBuffer that provides
an array accessor method that would allow them to be access as [1, 2].


For examples consult Test.cpp which contains a number of use cases.
