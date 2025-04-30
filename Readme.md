# Echo Server

**Echo Server** is a server application that returns _anything you've sent_ back to the client.

## Supported platform

Support Windows only.

## How to build from source

This project is based on **CMake**, so it is recommended to build it using **CMake**, with GCC (MSYS2, UCRT64) or MSVC as the compiler.

Clone the repository, run CMake in the root dir, and you will get cli.exe and svr.exe.

## Plans & goals

This repo is for learning purposes; advanced features like multi-threading, multi-clients and even GUI would be implemented successively in the near future.

The final goal is to develop a C/S LAN chat application.