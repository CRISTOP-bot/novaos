#!/usr/bin/env python3
import struct, sys
elf=open(sys.argv[1],'rb').read()
path=b'/init\0'+b'\0'*59
header_size=32+80
# magic, version, count, header_size, data_size; entry path, offset, size
with open(sys.argv[2],'wb') as f:
 f.write(struct.pack('<QIIQQ',0x53465241564f4e41,1,1,header_size,len(elf)))
 f.write(struct.pack('<64sQQ',path,header_size,len(elf)))
 f.write(elf)
