#!/usr/bin/env python3
import struct, sys
out=sys.argv[-1]
pairs=sys.argv[1:-1]
assert len(pairs)%2==0
items=[]
for i in range(0,len(pairs),2): items.append((pairs[i].encode(),open(pairs[i+1],'rb').read()))
header_size=32+80*len(items)
offset=header_size
with open(out,'wb') as f:
 f.write(struct.pack('<QIIQQ',0x53465241564f4e41,1,len(items),header_size,sum(len(x[1]) for x in items)))
 for path,data in items:
  f.write(struct.pack('<64sQQ',path+b'\0'*(64-len(path)),offset,len(data)));offset+=len(data)
 for _,data in items:f.write(data)
