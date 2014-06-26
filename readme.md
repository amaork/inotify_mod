#inotify_mod

inotify_mod is a little file watching utility, currently only support Linux, it works base on linux inotify (monitoring file system events) facility.

## Purpose

inotify_mod use to watch some specified file or directory, when the watching file or directory has changed it will notice user or log it.

## Feature

- Support : add, delete, modify, read event
- Support watching multi files or directory
- Using UDP message report watching events
- Message data can be Ascii data or binary data
- Support dynamic adding watching (When file/directory is created automatically added to watching list)

## Events

#### Directory events

- add:	Add subfile or subdirectory in watched directory 
- del:	Delete subfile or subdirectory in watched directory
- mod:	Equal to add | del events
- sdel:	Watching directory was deleted
- all:	All above events

#### File events

- mod:	File for write was closed
- read:	File for read was closed
- sdel:	Watching file was deleted
- all:	All above events

## Dynamic adding feature

For instance we want watching "/tmp/test" path, but currently its not exist, in usual add an un exist path to inotify it will cause an error. Use dynamic adding feature it can detect '"/tmp/test" path is not exist, then automatically add "/tmp" dir ("/tmp/test" father dir)  to inotify and watching  it's add event. When "test" file  is created in "/tmp" dir, app will automatically  add "/tmp/test" to inotify replace watching it's father dir "/tmp".

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

	[TEST]

		path	=	/tmp/test
		events	=	[all]
		comment	=	"Test"


it means watching `/tmp` directory, when it has `add` or `self delete` events happened, it will send ASCII message to multicast group `224.0.0.15:8900`.  Especially when new added subfile or subdirectory name is `a b c d e f g h` , it will set the specially file mask in send message .




## What's next?
- Support log file
- Support more event
- Support tcp message report 
- Support dynamic add file or directory  
- Support watch subdirectories with specified depth