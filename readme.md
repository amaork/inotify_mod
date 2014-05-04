#inotify_mod

inotify_mod is a little file watching utility, currently only support Linux, it works base on linux inotify (monitoring file system events) facility.

## Purpose

inotify_mod use to watch some specified file or directory, when the watching file or directory has changed it will notice user or log it.

## Feature

- Support : add, delete, modify event
- Support watching multi files or directory
- Using UDP message report watching events
- Message data can be Ascii data or binary data

## System request

Linux kernel >= 2.6.15

## Build

make

## Usage

1. First create a configure file "inotify_mod.ini".
2. Second start inotify_mod apps "./inotify\_mod".
3. Start a UDP server receive inotify event and process it.

**Notice:** 

1. Configure file format please refer source code dir "inotify_mod.ini".
2. You can Specify a configure file name with -F option, when start inotify_mod.  

## What's next?
- Support log file
- Support more event
- Support tcp message report 
- Support dynamic add file or directory  
- Support watch sub directiry with specify depth