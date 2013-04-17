Network Code Module
===

Quick info
---

This is a kernel module that implements "network code", a language that allows the description and verification of real time network communication. It uses sysfs for communication with userspace. It currently bypasses the socket abstraction for sending packets and writes directly to the driver. However, it uses raw sockets for receiving packets. We are trying to move to a lower level interface for receiving packets in real time.

Warning: In an ideal world we would 'install' the network code module tool chain: scripts for loading, starting, stopping, etc. but right now the scripts have some paths hardcoded, so they depend on being in their present directories.

Look at example.sh for a typical usage example.

Requirements
---

For the network code module to compile:

- make
- rt linux 3.6.11 (assumed to be in `/usr/src/linux-3.6.11-rt` - change it in the makefile if it's somewhere else)

To use the added tools:

- Python 2 or 3

To run the visualization:

- Python 2
- A modern web browser (works best in Chrome)

Usage
---

`make all install` to compile and load the module.

Look at the directories in `/sys/network_code` when the module is loaded to get an idea of what it can do.

To run a network code program you must write its contents to `/sys/network_code/code` and its parameters file to `/sys/network_code/params`. The parameter file consists of a list of mac addresses and device names that specify which peer this machine should be listening to / sending to and on which device. All packets are broadcast, so the mac is not used for sending, but it is used to filter incoming packets. After writing any commands to the network code module check `dmesg` for errors.

Write `run` or `stop` to `/sys/network_code/control` to start and stop the network code program after its code and params were written successfully.

`load.sh` is a script written to conveniently handle all of translating, loading and starting of a network code program. Take a look at `exmaple.sh` for usage information.

Translator
---

Note that this module uses its own format for network code instructions, so you need to make sure you use `ncm_translator` if you want to use the bytecode from the hardware implementation.

`ncm_translator/ncm_translator` uses standard in and standard out. You can feed it any network code program in the 'hardware' format and it will output a network code program in our format.

Parameters
---

`ncm_translator/mkparams.py` is a python script (compatible with both python 2 and 3) that takes in a json definition of the parameters to the network code module and turns it onto a binary one that can be written to the `params` sysfs entry. The format looks like this:

    {
        "channels": [
            {
                "mac": "08:00:27:C0:56:5B",
                "dev": "eth0"
            }
        ]
    }

Visualization
---

`ncm_visualizer/parse.py` is a python 2 script that takes in (on standard in) a network code program in our format (programs form the hardware format can be run through `ncm_translator`) and it performs preprocessing on it to prepare it to be displayed by `visualize.html`. `ncm_visualizer/test.sh` has an example of how one might use it. The output is a json. The input to `visualize.html` is an object of this form:

    {
        "ncm_program1": <json from parse.py>,
        "ncm_program2": <json from parse.py>
    }

`ncm_visualizer/data.json` needs to call `receive_data` using the object described above, so its contents would look like:

    receive_data({
        "ncm_program1": <json from parse.py>,
        "ncm_program2": <json from parse.py>
    });

Extra
---

Check out `compile_all_params.sh` and `compile_bytecode.sh` in the samples directory for other useful usage examples for converting between formats.

The two network code programs (rx1 and tx1) in the samples directory are a good test of the system. Take a look at using `send.sh` and `receive.sh` to simulate reads and writes to the variable space.

Almost undocumented features
---

`hax.h` was used to build the module with a preloaded network code program. If you are trying to use it, compile with `make vm=1` and load the module with `make install vm=1`.