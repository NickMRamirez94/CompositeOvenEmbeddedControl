# Composite Oven Embedded Controller
The Embedded Controller application is part of the User Programmable Composite Oven project. This project was completed by Chad Palmer (Lead Designer of this embedded application), Nicholas Ramirez, and David Brassfield. The application is meant to be used alongside the [Cure Cycle Application](https://github.com/NickMRamirez94/CompositeOvenGUI). The embedded controller controls the oven by running the cure cycles from an SD card and saving the collected data back to the card.

### Development
Created using [Platform.IO](https://platformio.org/) for use on an ATMEGA1284P microcontroller.

## Requirements

Software
* Ubuntu 16.04+ (or equivalent Linux distro)
* Visual Studio Code with Platform.IO
* Git

Hardware
* Atmega 1284P Microcontroller with Arduino Bootloader
* SD card reader
* Buttons
* LCD Screen

## Installation of requirements
### Using Web Installer

[Visual Studio Code Download](https://code.visualstudio.com/download) - (IDE)

[Platform IO](https://platformio.org/) - Can be installed as a plugin from VSC

### Install the Code

Download the contents of this repository as a .zip file and unzip it into a directory of your choice. Open Visual Studio Code and select "Open from Directory" in the "File" pull-down menu. Open the directory you saved the program to.

### Building the Application

From within Visual Studio, press CTRL-ALT-B

## Run the Application

Once the program is installed into the microcontroller, it should run at startup.

## How to Use

Upon launching the application a user friendly menu system will guide you through the necessary steps for uploading and running a cure cycle.
