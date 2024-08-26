<p align="center">
	<a href="https://crossdb.org">
	<img src="https://crossdb.org/assets/favicon.png">
	</a>
</p>
<p align="center">
	<strong>Super High-performance Lightweight Embedded and Server OLTP RDBMS✨</strong>
</p>

> **NOTE** 
> This project was redesigned and rewritten from scratch.
> It's still in early development stage, so please DO NOT use in your project now.

**CrossDB** is a super high-performance lightweight embedded and server OLTP RDBMS. 
It's developed for high performance scenarios with main memory can hold whole DB.


## Features

- Support Multiple OS Platforms: Linux/Windows/MacOS/FreeBSD etc
- Support Multiple CPU ARCH: X86/ARM/PPC/MIPS etc
- Support OnDisk/In-memory(IMDB)/RamDisk/Hybrid Storage
- Support Standard RDBMS model
- Support Standard SQL and many extensions from MySQL
- Support Multiple databases
- Support Embedded and Client-Server mode(TBD)
- Support Primary Key and multiple Secondary Indexes
- Support HASH and RBTREE(TBD) Index
- Support Multi-columns Index
- Support Exact Match, Leftmost Match(TBD), Range Match(TBD)
- Support Standard ACID Transaction (begin/commit/rollback)
- Support WAL for OnDisk storage(TBD)
- Support Multiple Threads and Multiple Processes Access
- Support Table level read-write lock
- Support Reader-Writer MVCC
- Support Embedded CrossDB Shell with convenient auto-completion
- Support Multi-Statments APIs
- Support Prepared Statments APIs
- Super High Performance
- Very Simple: Simple header and library file
- Zero Config: no complex config, real out-of-the-box


## Use Cases

- High-frenquency trade (OLTP)
- High-peformance query
- High-peformance data manamgent: You can use CrossDB OnDisk DB to store data on Disk/Flash/SSD.
- High-peformance IMDB: You can use CrossDB In-Memory DB to manage Process Runtime Data to replace STL or hand-written data structures.
- You can use CrossDB RamDisk DB to support Process Restartability, In-Service Software Upgrade(ISSU) easily.
- You can use CrossDB to work as a super fast cache DB.


## Build and Install

### Linux/FreeBSD

```bash
make build
sudo make install
```

### MacOS

```bash
make build
sudo make installmac
```

### Windows

You need to install [MINGW64](https://www.mingw-w64.org/) to build.
Then set `gcc` path to `system environment variables` `Path` and make sure `gcc` can run.

```
winbuild.bat
```

## Contribution

Following contributions are welcome:

- Language bindings: `Python`, `Java`, `Go`, `CSharp`, `Javascript`, `PHP`, etc
- Test and report bugs


## Reference

### SQL Statements

https://crossdb.org/sql/statements/

### APIs

https://crossdb.org/client/api-c/

### Tutorial

https://crossdb.org/get-started/tutorial/
