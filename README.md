# ticketmaster-but-worse
A project for the binary hacking practical

## instructions
### Clone repo:
`git clone --recurse-submodules git@github.com:JulianFP/ticketmaster-but-worse.git`


### Start flask dev server:

`python3 -m venv .venv`

`source .venv/bin/activate`

`pip install .`

`cd ticketmaster-but-worse`

`flask run`


### Compile and run client:

**Qt5 development tools**:
    install qtbase5-dev qt5-qmake qtbase5-dev-tools
    install libpng-dev

**CMake and build-essential**:
    install cmake build-essential



`cmake -B build`

`make -C build`

`./build/ticketmaster-but-worse`
