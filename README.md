Stormwater-Management-Model
===========================

ORD Stormwater Management Model (aka "SWMM")

Introduction
------------
This is the official SWMM source code repository maintained by US EPA ORD, NRMRL, Water Supply and Water Resources Division located in Cincinnati, Ohio.

SWMM is a dynamic hydrology-hydraulic water quality simulation model. It is used for single event or long-term (continuous) simulation of runoff quantity and quality from primarily urban areas. SWMM source code is written in the C Programming Language and released in the Public Domain.

Building With Docker
-------------

    docker build -t usepa-stormwater-management-model .

Running Supplied Example With Docker
-------------

    docker run --rm -it usepa-stormwater-management-model swmm5 example/parkinglot.inp example/parkinglot.rpt example/parkinglot.out; cat example/parkinglot.rpt

Find Out More
-------------
The source code distributed here is identical to the code found at the official [SWMM Website](http://www2.epa.gov/water-research/storm-water-management-model-swmm). 
