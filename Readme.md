# About Echo

**Echo** is a simple IM (instant messaging) application using client/server model.

The name **Echo** comes from the prototype of this project, in which the messages sent to the server are simply sent back (*echo*ed).

## Supported platform

Support Windows only.

## Usage

### Server

```shell
\path\to\svr.exe <ip> <port>
```
ip: The IPv4 address that the server wants to listen on (or `0.0.0.0` for every local IP).

port: The local port that the server wants to listen on (0-65535).

### Client

```shell
\path\to\cli.exe <ip> <port>
```
ip: The IPv4 address (of a server) that the client wants to connect to.

port: The port of the server (0-65535).

## How to build from source

This project is based on **CMake**, so it is recommended to build it using **CMake**, with GCC (MSYS2, UCRT64) or MSVC as the compiler.

Clone the repository, run CMake in the root dir, and you will get cli.exe and svr.exe.

## Plans & goals

This repo is for learning purposes.

Codes will be separated into functions of APIs in the near future.

GUI is not planned.