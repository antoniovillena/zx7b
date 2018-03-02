This is basically a port of ZX7 by Einar Saukas

http://www.worldofspectrum.org/infoseekid.cgi?id=0027996

Follow the file example.asm. Assembled with sjasmplus. Benchmark made with Ticks.
Also attached tap8k utility to generate .TAP file.

The decrounching algorithm is backwards so load HL and DE pointing to the end of
the compressed and uncompressed buffers respectively.

The next table shows performance for different routines.

                slow   medium    fast0    fast1    fast2
--------------------------------------------------------
lena1k        136724   112287    83707    81511    81040
lena16k      2578233  2091259  1518892  1468437  1462568
lena32k      4974712  4023756  2913537  2818843  2803116
alice1k       115167    95877    75410    73616    73459
alice16k     2158335  1769433  1366443  1326895  1328886
alice32k     4312995  3534254  2730160  2649630  2654236
128rom1k      103020    83629    64069    62825    62000
128rom16k    1970827  1605062  1219271  1188707  1180569
128rom32k    4003487  3257233  2463147  2399125  2381847
--------------------------------------------------------
routine size      64       67      156      177      191

The next one compares compression ratio with other algorithms.
Numbers are filesizes in bytes.

Size    Shrinkler Exomizer    aPLib   saukav     zx7b  BBuster 
lena1k        776      812      872      873      902      905
lena16k     13796    13581    14635    14649    14689    14798
lena32k     28368    28019    29991    30071    30272    30446
alice1k       548      613      617      611      631      636
alice16k     6872     7266     7659     7738     8175     8429
alice32k    12868    13461    14473    14535    16074    16570
128rom1k      840      884      889      913      923      925
128rom16k   11848    12260    12434    12728    12806    12882
128rom32k   23648    24415    24820    26157    26524    26708

This last table compares speed with other algorithms.
Numbers are execution cycles.

           Shrinkler  deexov4    aPLib  BBuster  zx7mega   saukav   zx7bf2
--------------------------------------------------------------------------
lena1k      13757267   303436   176642   106746    95255    76547    81040
lena16k    238317371  4407913  2961621  1908398  1727095  1646032  1462568
lena32k    484967405  8443253  5820921  3651800  3300486  3231882  2803116
alice1k     10060954   274111   136224    98914    89385    70869    73459
alice16k   131592504  2973592  2143122  1812259  1614225  1338287  1328886
alice32k   249719379  5378511  4189855  3614393  3230255  2550243  2654236
128rom1k    13773150   249124   131667    82637    74110    60222    62000
128rom16k  197319929  3571407  2292945  1550682  1407478  1392317  1180569
128rom32k  394594060  7355277  4583902  3107867  2825773  1926027  2381847
--------------------------------------------------------------------------
routine size     245      201      197      168      244     ~200      191

saukav is a generalization for zx7b algorithm. If you are a experienced
user I recommend to read saukav.txt and use saukav.c/exe in this folder.

Created by Antonio José Villena Godoy, 2017
Creative Commons Licensed by SA
https://creativecommons.org/licenses/by-sa/4.0/legalcode
