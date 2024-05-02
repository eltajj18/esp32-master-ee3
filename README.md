# ESP32 Master Node Project
### ESP-IDF Project to communicate the score or next move to make in the game of tic-tac-toe to STM8 slave nodes through NRF using minimax algorithm, HTTP requests

## Example folder contents
ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
    ├── include
        ├── game_config.h
        ├── minimax.h
        ├── http_request.h
        ├── mirf.h
        ├── nrf_configuration.h
        ├── wifi_connection.h
│   ├── CMakeLists.txt
│   ├── main.c
│   ├── minimax.c
    ├── game_config.c
    ├── http_request.c
    ├── mirf.c
    ├── nrf_configuration.c
   
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
## Functionalities
* NRF communication between ESP32 and STM8
* Minimax Algorithm to calculate best move to make in the game of tic-tac-toe
* Wi-Fi Connection
* HTTP GET and HTTP POST and Polling requests implementation from and to FastAPI server
## Flow Chart
![EE3_ESP32 - Page 2](https://github.com/eltajj18/esp32-master-ee3/assets/100543589/d09cf5f7-1c9e-4e4d-9873-914cff21efb9)

## Resources
* I followed a tutorial from<a href = "https://www.geeksforgeeks.org/finding-optimal-move-in-tic-tac-toe-using-minimax-algorithm-in-game-theory/
"> geeksforgeeks </a> for part of the minimax alghorithm
* All the credit for the NRF library goes to<a href = "https://github.com/nopnop2002/esp-idf-mirf"> nopnop20020 </a>
