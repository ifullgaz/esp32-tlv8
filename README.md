esp32-tlv8
============

esp32-tlv8 is a minimal C implementation of a tlv structure with 8 bit type and 8 bit size fields.
It can be dropped as is as a component in an ESP-IDF project.

Installation
------------

esp32-tlv8 is now dependant on [esp32-utils] (https://github.com/ifullgaz/esp32-utils).  
Clone the module directly into the components directory of your project.

```git clone https://github.com/ifullgaz/esp32-tlv8```

To run the test, also clone the dependancies into the components directory of the test app

```git submodule update --init```

Test
----

The test can be ran from the command line as a normal ESP-IDF project.
Simply run ```make flash monitor``` to run on a connected esp32 board.

Usage
-----

Please look at the test source file for an example of using tlv structure