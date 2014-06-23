#inotify_mod

inotify_mod is a little file watching utility, currently only support Linux, it works base on linux inotify (monitoring file system events) facility.

## Purpose

inotify_mod use to watch some specified file or directory, when the watching file or directory has changed it will notice user or log it.

## Feature

- Support : add, delete, modify event
- Support watching multi files or directory
- Using UDP message report watching events
- Message data can be Ascii data or binary data

## Events

#### Directory events

- add:	Add subfile or subdirectory in watched directory 
- del:	Delete subfile or subdirectory in watched directory
- mod:	Equal to add | del events
- sdel:	Watching directory was deleted

#### File events

- mod:	File for write was closed
- read:	File for read was closed
- sdel:	Watching file was deleted

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

## Example

`inotify_mod.ini`:

	port 	= 8900			;comm port
	addr 	= 224.0.0.15	;report address
	ascii	= yes			;yes = ascii, no = binary

	[TMP]
	
		path	=	/tmp
		events	=	[add, sdel]
		special	=	[a,b,c,d,e,f,g,h]
		comment	=	"System /tmp directory"


it means watching `/tmp` directory, when it has `add` or `self delete` events happened, it will send ASCII message to multicast group `224.0.0.15:8900`.  Especially when new added subfile or subdirectory name is `a b c d e f g h` , it will set the specially file mask in send message .


## What's next?
- Support log file
- Support more event
- Support tcp message report 
- Support dynamic add file or directory  
- Support watch subdirectories with specified depth