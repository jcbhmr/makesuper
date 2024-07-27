# SuperApp

## Installation

## Usage

```
.
├── LINUX-X86_64/
│   └── cc
├── LINUX-AARCH64/
│   └── cc
├── XNU-X86_64/
│   └── cc
├── XNU-AARCH64/
│   └── cc
├── WINDOWS-X86_64/
│   └── cc.exe
└── makesuper.txt
```

<div><code>makesuper.txt</code></div>

```
supercc
LINUX-X86_64   cc
LINUX-AARCH64  cc
XNU-X86_64     cc
XNU-AARCH64    cc
WINDOWS-X86_64 cc
```

```
cp superapp.com supercc
zip -Ar supercc .
```
