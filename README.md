# LemonDBFS

A fuse file system cooperated with [LemonDB](https://github.com/Reapor-Yurnero/LemonDB) to handle queries and tables. (extension of VE482 Lab8)

## Usage

Compile:
- `cd src`
- `make`
- `cd ..`

Run & Test:
- `./src/lemondbfs -f dbdir/ mountdir/`
- `ls -laR mountdir/`
- `cd mountdir/mTable0/`
- `echo "SWAP ( c16 c9 ) FROM mTable0 WHERE ( c0 <= -20302 ) ;" > .query`
- `ls -a`
- `head res.tbl`
- `cd ../..`
- `fusermount -u mountdir`

## Design Strategy

Currently no real file system operation is operated on mountdir/. Instead, they are all handled/translated/mirrored to real local file system operations or LemonDB instructions acted on dbdir/ and tables/files inside.

## Overview

Goals: 
- access lemonDB files and run queries without accessing the software
- extend UNIX "everything is a file" philosophy: a query is a file

Characteristics of lemonDBfs:
- usage: lemondbfs db_dir mount_dir

- strategy:
  1. for each .tbl file show a dir in mount_dir
     eg. for file mTable10.tbl show dir mTable10

  2. inside each dir show the corresponding table in file orig.tbl
     eg. for file mTable10.tbl show mtable10/orig.tbl
  
  3. inside each dir show an empty file called .query
     eg. for mTable10.tbl show the empty file mTable10/.query
  
  4. when a query is written to a .query file, apply it the orig.tbl
     and create the file res.tbl containing the result of the query
		 eg. upon a write on mTable10/.query run the query (error on wrong
		 query) and create mTable10/res.tbl with the result of the query


# Development

Getting started:
- prepare a skeleton of the code (eg. based on hello.c example?)
- add a simple printf to understand how/when the functions are called
  eg. `printf("getattr(\"%s\"\n", path);`
- keep the code short and simple, no long functions

Useful resources:
- libfuse source code: https://github.com/libfuse/libfuse
- fuse documentation: https://libfuse.github.io/doxygen/

- real examples: 
  * fuse examples: https://github.com/libfuse/libfuse/tree/master/example
  * sshfs: https://github.com/libfuse/sshfs/blob/master/sshfs.c
  * fuse tutorial (the most useful one): https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/	

- toy examples:
  * https://engineering.facile.it/blog/eng/write-filesystem-fuse/
  * http://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/

## TODO

- lemonDBfs must be more stable (cf. BUGS)

- Allow more than one query in the .query file 

- When many queries are input allow temporary result tables to be 
  created. Use a mount flag? Take advantage of the DUMP query or use
  another approach?

- Allow the creation .tbl files: a dir in mount_dir + orig.tbl + .query file should create a corresponding .tbl file in db_dir

- Allow the deletion of .tbl files: when a dir is deleted in mount_dir
  the corresponding .tbl file should disappear

- Update this TODO. Creation + deletion are on the way, but not stable
  at all yet

# BUGS

!!DO NOT RUN AS ROOT!!

Could potentially corrupt the DB, or even crash lemonDB. New creation 
and deletion are very unstable and can have terrible consequences...

*Already handled by uid check to forbid root user.*
