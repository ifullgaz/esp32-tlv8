esp32-tlv8
============

esp32-tlv8 is a minimal C implementation of a tlv structure with 8 bit type and 8 bit size fields.
It can be dropped as is as a component in an ESP-IDF project.

Installation
------------

Extract or clone the module directly into the components directory of your project

Test
----

The test can be ran from the command line as a normal ESP-IDF project.
Simply run ```make flash``` to run on a connected esp32 board.

Usage
-----

Please look at the test source file for an example of using tlv structure