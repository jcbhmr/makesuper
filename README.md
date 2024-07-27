# SuperApp

## Installation

## Usage

```
.
├── LINUX-X86_64/
│   └── myapp
├── LINUX-AARCH64/
│   └── myapp
├── XNU-X86_64/
│   └── myapp
├── XNU-AARCH64/
│   └── myapp
├── WINDOWS-X86_64/
│   └── myapp.exe
└── superapp.txt
```

<div><code>superapp.txt</code></div>

```
myapp
LINUX-X86_64   myapp
LINUX-AARCH64  myapp
XNU-X86_64     myapp
XNU-AARCH64    myapp
WINDOWS-X86_64 myapp.exe
```

```
cp superapp myapp
cd zip
zip -Ar ../myapp .
```
