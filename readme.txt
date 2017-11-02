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

Size      Exomizer    aPLib   saukav     zx7b  BBuster
lena1k         812      872      873      902      905
lena16k      13581    14635    14649    14689    14798
lena32k      28019    29991    30071    30272    30446
alice1k        613      617      611      631      636
alice16k      7266     7659     7738     8175     8429
alice32k     13461    14473    14535    16074    16570
128rom1k       884      889      913      923      925
128rom16k    12260    12434    12728    12806    12882
128rom32k    24415    24820    26157    26524    26708

This last table compares speed with other algorithms.
Numbers are execution cycles.

             deexov4    aPLib  BBuster  zx7mega   saukav   zx7bf2
-----------------------------------------------------------------
lena1k        303436   176642   106746    95255    78447    81040
lena16k      4407913  2961621  1908398  1727095  1711409  1462568
lena32k      8443253  5820921  3651800  3300486  3360758  2803116
alice1k       274111   136224    98914    89385    72388    73459
alice16k     2973592  2143122  1812259  1614225  1390629  1328886
alice32k     5378511  4189855  3614393  3230255  2649718  2654236
128rom1k      249124   131667    82637    74110    61562    62000
128rom16k    3571407  2292945  1550682  1407478  1443539  1180569
128rom32k    7355277  4583902  3107867  2825773  2272056  2381847
-----------------------------------------------------------------
routine size     201      197      168      244     ~200      191


Created by Antonio José Villena Godoy, 2017
Creative Commons Licensed by SA
https://creativecommons.org/licenses/by-sa/4.0/legalcode
