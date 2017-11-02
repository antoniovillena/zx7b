/*
 * saukav (c) Copyright 2017 by Antonio Villena. All rights reserved.
 *
 * Based on ZX7 <http://www.worldofspectrum.org/infoseekid.cgi?id=0027996>
 * (c) Copyright 2012 by Einar Saukas. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of its author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILESIZE 65536

typedef struct match_t {
  size_t index;
  struct match_t *next;
} Match;

typedef struct optimal_t {
  size_t bits;
  int offset;
  int len;
} Optimal;

int off_bits, max_offset, max_len, bit_mask;
Match *matches, *match_slots;
Optimal *optimal;
size_t *min, *max, input_size, output_size, output_index, bit_index;
unsigned char *output_data, *input_data;
int (*eliasbits)(int);
void (*eliaswrite)(int);

int count_bits(int offset, int len) {
    return 1 + (offset > 128 ? off_bits : 8) + eliasbits(len-1);
}

void write_byte(int value) {
  output_data[output_index++] = value;
}

void write_bit(int value) {
  if( bit_mask == 0 )
    bit_mask = 128,
    bit_index = output_index,
    write_byte(0);
  if( value > 0 )
    output_data[bit_index] |= bit_mask;
  bit_mask >>= 1;
}

void compress() {
  size_t input_index;
  size_t input_prev;
  int offset1;
  int mask;
  int i;

  /* calculate and allocate output buffer */
  input_index= input_size-1;
  output_size= (optimal[input_index].bits+16+7)/8;

  /* un-reverse optimal sequence */
  optimal[input_index].bits= 0;
  while ( input_index > 0 )
    input_prev= input_index - (optimal[input_index].len > 0 ? optimal[input_index].len : 1),
    optimal[input_prev].bits= input_index,
    input_index= input_prev;

  output_index= 0;
  bit_mask= 0;

  /* first byte is always literal */
  write_byte(input_data[0]);

  /* process remaining bytes */
  while ( (input_index= optimal[input_index].bits) > 0)
    if( optimal[input_index].len == 0)
      write_bit(0),
      write_byte(input_data[input_index]);
    else{
      /* sequence indicator */
      write_bit(1);

      /* sequence length */
      eliaswrite(optimal[input_index].len-1);

      /* sequence offset */
      offset1= optimal[input_index].offset-1;
      if( offset1 < 128 )
        write_byte(offset1);
      else{
        offset1-= 128;
        write_byte((offset1 & 127) | 128);
        for ( mask= 1<<(off_bits-2); mask > 127; mask >>= 1)
          write_bit(offset1 & mask);
      }
    }

  /* end mark */
  write_bit(1);
  eliaswrite(0xff);
}

void optimize() {
  Match *match;
  int match_index;
  int offset;
  size_t len;
  size_t best_len;
  size_t bits;
  size_t i;

  memset(min, 0, (max_offset+1)*sizeof(size_t));
  memset(max, 0, (max_offset+1)*sizeof(size_t));
  memset(matches, 0, 256*256*sizeof(Match));
  memset(match_slots, 0, input_size*sizeof(Match));
  memset(optimal, 0, input_size*sizeof(Optimal));
  optimal[0].bits = 8;
  for ( i= 1; i < input_size; i++ ){
    optimal[i].bits= optimal[i-1].bits + 9;
    match_index= input_data[i-1] << 8 | input_data[i];
    best_len= 1;
    for ( match= &matches[match_index]; match->next != NULL && best_len < max_len; match = match->next){
      offset= i - match->next->index;
      if( offset > max_offset ){
        match->next = NULL;
        break;
      }
      for ( len= 2; len <= max_len; len++ ){
        if( len > best_len && len&0xff ){
          best_len= len;
          bits= optimal[i-len].bits + count_bits(offset, len);
          if( optimal[i].bits > bits )
            optimal[i].bits= bits,
            optimal[i].offset= offset,
            optimal[i].len= len;
        }
        else if ( i+1 == max[offset]+len && max[offset] != 0 ){
          len= i-min[offset];
          if( len > best_len )
            len= best_len;
        }
        if( i < offset+len || input_data[i-len] != input_data[i-len-offset] )
          break;
      }
      min[offset]= i+1-len;
      max[offset]= i;
    }
    match_slots[i].index= i;
    match_slots[i].next= matches[match_index].next;
    matches[match_index].next= &match_slots[i];
  }
}

int elias_gamma_bits1(int value) {
  int bits= 1;
  while ( value > 1 )
    bits+= 2,
    value>>= 1;
  return bits;
}

int elias_gamma_bits2(int value) {
  int bits= 2;
  --value;
  while ( value > 1 )
    bits+= 2,
    value-= 2,
    value>>= 1;
  return bits;
}

void write_elias_gamma1(int value) {
  int bits= 0, rvalue= 0;
  while ( value>1 )
    ++bits,
    rvalue<<= 1,
    rvalue|= value&1,
    value>>= 1;
  while ( bits-- )
    write_bit(0),
    write_bit(rvalue & 1),
    rvalue>>= 1;
  write_bit(1);
}

void write_elias_gamma2(int value) {
  int bits= 0, rvalue= 0;
  --value;
  while ( value>1 )
    ++bits,
    value-= 2,
    rvalue<<= 1,
    rvalue|= value&1,
    value>>= 1;
  rvalue<<= 1,
  rvalue|= value&1;
  write_bit(rvalue & 1);
  while ( bits-- )
    rvalue>>= 1,
    write_bit(0),
    write_bit(rvalue & 1);
  write_bit(1);
}

int main(int argc, char *argv[]) {
  FILE *ifp, *ofp;
  size_t partial_counter, total_counter;
  char *output_name, type, back, speed;
  int i, j, fil, size[18];

  if( argc==1 )
    printf("\nsaukav compressor v1.00 by Einar Saukas/AntonioVillena, 1 Nov 2017\n\n"
           "  saukav <type> <file1> <file2> .. <fileN>\n\n"
           "  zx7b <input_file> <output_file>\n\n"
           "  <type>           Target decruncher\n"
           "  <file1..N>       Origin files\n\n"
           "Valid <type> values are: f0, f1, f2, f3, f4, b0, b1, b2, b3 and b4\n"
           "Every input file will be compressed in a .skv output file\n"
           "It will generate the decruncher into the file d.asm\n\n"),
    exit(0);
  if( argc<3 )
    printf("\nInvalid number of parameters\n"),
    exit(-1);
  back= (~argv[1][0] & 4) >> 2;
  speed= argv[1][1] - '0';
  matches= (Match *)calloc(256*256, sizeof(Match));
  output_data= (unsigned char *)malloc(MAX_FILESIZE);
  input_data= (unsigned char *)malloc(MAX_FILESIZE);
  match_slots= (Match *)calloc(MAX_FILESIZE, sizeof(Match));
  optimal= (Optimal *)calloc(MAX_FILESIZE, sizeof(Optimal));
  min= (size_t *)calloc(65537, sizeof(size_t));
  max= (size_t *)calloc(65537, sizeof(size_t));
  if( !matches || !output_data || !input_data || !match_slots
      || !optimal || !min || !max )
    fprintf(stderr, "Error: Insufficient memory\n"),
    exit(1);
  for ( off_bits= 0; off_bits<17; off_bits++ )
    size[off_bits]= 0;
  for (fil= 2; fil < argc; fil++) {
    ifp= fopen(argv[fil], "rb");
    if( !ifp )
      fprintf(stderr, "Error: Cannot access input file %s\n", argv[1]),
      exit(1);
    fseek(ifp, 0L, SEEK_END);
    input_size= ftell(ifp);
    fseek(ifp, 0L, SEEK_SET);
    if( !input_size )
      fprintf(stderr, "Error: Empty input file %s\n", argv[1]),
      exit(1);
    total_counter= 0;
    do {
      partial_counter = fread(input_data+total_counter, sizeof(char), input_size-total_counter, ifp);
      total_counter += partial_counter;
    } while ( partial_counter > 0 );
    if( total_counter != input_size )
      fprintf(stderr, "Error: Cannot read input file %s\n", argv[1]),
      exit(1);
    fclose(ifp);
    if (back)
      for ( i= 0; i<input_size>>1; i++ )
        j= input_data[i],
        input_data[i]= input_data[input_size-1-i],
        input_data[input_size-1-i]= j;
    for ( off_bits= 8; off_bits<17; off_bits++ )
      max_offset= (1<<(off_bits-1))+128,
      max_len= off_bits==8 ? 256 : 65536,
      eliasbits= &elias_gamma_bits1,
      eliaswrite= &write_elias_gamma1,
      optimize(),
      size[off_bits-8<<1]+= optimal[input_size-1].bits+23>>3,
      eliasbits= &elias_gamma_bits2,
      eliaswrite= &write_elias_gamma2,
      optimize(),
      size[off_bits-8<<1|1]+= optimal[input_size-1].bits+23>>3;
  }
  for ( i= 2e9, j= 0; j<18; j++ )
    if( i>size[j] )
      i= size[j],
      type= j;
  off_bits= type+16>>1;
  max_offset= (1<<(off_bits-1))+128;
  max_len= off_bits==8 ? 256 : 65536;
  eliasbits= type&1 ? elias_gamma_bits2 : elias_gamma_bits1;
  eliaswrite= type&1 ? write_elias_gamma2 : write_elias_gamma1;
  for (fil= 2; fil < argc; fil++) {
    ifp= fopen(argv[fil], "rb");
    fseek(ifp, 0L, SEEK_END);
    input_size= ftell(ifp);
    fseek(ifp, 0L, SEEK_SET);
    total_counter= 0;
    do {
      partial_counter= fread(input_data+total_counter, sizeof(char), input_size-total_counter, ifp);
      total_counter+= partial_counter;
    } while ( partial_counter > 0 );
    fclose(ifp);
    output_name= (char *)malloc(strlen(argv[fil]) + 5);
    strcpy(output_name, argv[fil]);
    strcat(output_name, ".skv");
    ofp= fopen(output_name, "wb+");
    if( !ofp )
      fprintf(stderr, "Error: Cannot create output file %s\n", argv[2]),
      exit(1);
    if (back)
      for ( i= 0; i<input_size>>1; i++ )
        j= input_data[i],
        input_data[i]= input_data[input_size-1-i],
        input_data[input_size-1-i]= j;
    optimize();
    compress();
    if (back)
      for ( i= 0; i<output_size>>1; i++ )
        j= output_data[i],
        output_data[i]= output_data[output_size-1-i],
        output_data[output_size-1-i]= j;
    if( fwrite(output_data, sizeof(char), output_size, ofp) != output_size )
      fprintf(stderr, "Error: Cannot write output file %s\n", output_name),
      exit(1);
    fclose(ofp);
    printf("File %s compressed from %s (%d to %d bytes)\n", output_name, argv[fil], (int) input_size, (int) output_size);
  }
  ofp= fopen("d.asm", "wb+");
  if( !ofp )
    printf("\nCannot create d.asm file"),
    exit(-1);
  fprintf(ofp, "; %c%d o%d g%d\n", back ? 'b' : 'f', speed, off_bits, type&1);
  if( speed<2 || type<2 ){
    if( type&1 || type<2 )
      fprintf(ofp,     "sauk:   ld      a, 128\n"
                       "copyby: ld%c\n", back?'d':'i');
    else           
      fprintf(ofp,     "sauk:   ld      bc, 32768\n"
                       "        ld      a, b\n"
                       "copyby: inc     c\n"
                       "        ld%c\n", back?'d':'i');
    if( speed )
      fprintf(ofp,     "mainlo: add     a, a\n"
                       "        call    z, getbit\n");
    else
      fprintf(ofp,     "mainlo: call    getbit\n");
    fprintf(ofp,       "        jr      nc, copyby\n");
    if( type<2 ){
      if( type&1 ){
        fprintf(ofp,   "        ld      bc, 1\n");
        if( speed )
          fprintf(ofp, "lenval: add     a, a\n"
                       "        call    z, getbit\n");
        else
          fprintf(ofp, "lenval: call    getbit\n");
        fprintf(ofp,   "        rl      c\n"
                       "        ret     c\n");
        if( speed )
          fprintf(ofp, "        add     a, a\n"
                       "        call    z, getbit\n");
        else
          fprintf(ofp, "        call    getbit\n");
        fprintf(ofp,   "        jr      nc, lenval\n");
      }
      else{
        fprintf(ofp,   "        ld      bc, 0\n");
        if( speed )
          fprintf(ofp, "        defb    $30\n"
                       "lenval: add     a, a\n"
                       "        call    z, getbit\n");
        else
          fprintf(ofp, "lenval: call    nc, getbit\n");
        fprintf(ofp,   "        rl      c\n");
        if( speed )
          fprintf(ofp, "        add     a, a\n"
                       "        call    z, getbit\n");
        else
          fprintf(ofp, "        call    getbit\n");
        fprintf(ofp,   "        jr      nc, lenval\n"
                       "        inc     c\n"
                       "        ret     z\n");
      }
      fprintf(ofp,     "        push    hl\n"
                       "        ld      l, (hl)\n"
                       "        ld      h, b\n");
      if( back )
        fprintf(ofp,   "        adc     hl, de\n"
                       "        lddr\n"
                       "        pop     hl\n"
                       "        dec     hl\n"
                       "        jr      mainlo\n");
      else
        fprintf(ofp,   "        push    de\n"
                       "        ex      de, hl\n"
                       "        sbc     hl, de\n"
                       "        pop     de\n"
                       "        ldir\n"
                       "        pop     hl\n"
                       "        inc     hl\n"
                       "        jr      mainlo\n");
    }
    else{
      if( type&1 ){
        fprintf(ofp,   "        ld      bc, 1\n"
                       "        push    de\n"
                       "        ld      d, b\n");
        if( speed )
          fprintf(ofp, "lenval: add     a, a\n"
                       "        call    z, getbit\n");
        else
          fprintf(ofp, "lenval: call    getbit\n");
        fprintf(ofp,   "        rl      c\n"
                       "        jr      z, exitdz\n"
                       "        rl      b\n");
        if( speed )
          fprintf(ofp, "        add     a, a\n"
                       "        call    z, getbit\n");
        else
          fprintf(ofp, "        call    getbit\n");
        fprintf(ofp,   "        jr      nc, lenval\n");
      }
      else{
        fprintf(ofp,   "        push    de\n"
                       "        ld      d, c\n");
        if( speed )
          fprintf(ofp, "        defb    $30\n"
                       "lenval: add     a, a\n"
                       "        call    z, getbit\n");
        else
          fprintf(ofp, "lenval: call    nc, getbit\n");
        fprintf(ofp,   "        rl      c\n"
                       "        rl      b\n");
        if( speed )
          fprintf(ofp, "        add     a, a\n"
                       "        call    z, getbit\n");
        else
          fprintf(ofp, "        call    getbit\n");
        fprintf(ofp,   "        jr      nc, lenval\n"
                       "        inc     c\n"
                       "        jr      z, exitdz\n");
      }
      fprintf(ofp,     "        ld      e, (hl)\n"
                       "        %sc     hl\n"
                       "        defb    203, 51\n" // sll e
                       "        jr      nc, offend\n"
                       "        ld      d, %d\n", back?"de":"in", 1<<(17-type>>1));
      if( speed )
        fprintf(ofp,   "nexbit: add     a, a\n"
                       "        call    z, getbit\n");
      else
        fprintf(ofp,   "nexbit: call    getbit\n");
      fprintf(ofp,     "        rl      d\n"
                       "        jr      nc, nexbit\n"
                       "        inc     d\n"
                       "        srl     d\n"
                       "offend: rr      e\n"
                       "        ex      (sp), hl\n");
      if( back )
        fprintf(ofp,   "        ex      de, hl\n"
                       "        adc     hl, de\n"
                       "        lddr\n");
      else
        fprintf(ofp,   "        push    hl\n"
                       "        sbc     hl, de\n"
                       "        pop     de\n"
                       "        ldir\n");
      fprintf(ofp,     "exitdz: pop     hl\n"
                       "        jr      nc, mainlo\n");
    }
    if( speed )
      fprintf(ofp,     "getbit: ld      a, (hl)\n");
    else
      fprintf(ofp,     "getbit: add     a, a\n"
                       "        ret     nz\n"
                       "        ld      a, (hl)\n");
    fprintf(ofp,       "        %sc     hl\n"
                       "        adc     a, a\n"
                       "        ret\n", back?"de":"in");
  }
  else{
    fprintf(ofp,       "      macro getbitm\n"
                       "        add     a, a\n");
    if( speed==2 )
      fprintf(ofp,     "        call    z, getbit\n");
    else
      fprintf(ofp,     "        jp      nz, .gb1\n"
                       "        ld      a, (hl)\n"
                       "        %sc     hl\n"
                       "        adc     a, a\n"
                       ".gb1\n", back?"de":"in");
    fprintf(ofp,       "      endm\n"
                       "sauk:   ld      a, 128\n"
                       "copbye: ld%c\n"
                       "        add     a, a\n"
                       "        jr      z, mailao\n"
                       "        jr      c, maicoo\n"
                       "copbyo: ld%c\n"
                       "        add     a, a\n", back?'d':'i', back?'d':'i');
    if( speed>3 )
      fprintf(ofp,     "        jr      c, maicoe\n"
                       "        ld%c\n"
                       "        add     a, a\n"
                       "        jr      z, mailao\n"
                       "        jr      c, maicoo\n"
                       "        ld%c\n"
                       "        add     a, a\n", back?'d':'i', back?'d':'i');
    fprintf(ofp,       "        jr      nc, copbye\n");
    if( type&1 )
      fprintf(ofp,     "maicoe: ld      bc, 1\n"
                       "maisie: push    de\n"
                       "        ld      d, b\n"
                       "levale: getbitm\n"
                       "        rl      c\n"
                       "        jr      z, exitdz\n"
                       "        rl      b\n"
                       "        add     a, a\n"
                       "        jr      nc, levale\n"
                       "        ld      e, (hl)\n", speed==2 ? 'r' : 'p');
    else
      fprintf(ofp,     "maicoe: ld      bc, 2\n"
                       "maisie: push    de\n"
                       "        ld      d, b\n"
                       "        getbitm\n"
                       "        j%c      c, contie\n"
                       "        dec     c\n"
                       "levale: add     a, a\n"
                       "        rl      c\n"
                       "        rl      b\n"
                       "        getbitm\n"
                       "        jr      nc, levale\n"
                       "        inc     c\n"
                       "        jr      z, exitdz\n"
                       "contie: ld      e, (hl)\n", speed==2 ? 'r' : 'p');
    fprintf(ofp,       "        %sc     hl\n"
                       "        defb    203, 51\n" // sll e
                       "        jr      nc, offnd%c\n", back?"de":"in", type&1?'o':'e');
    for ( i=8; i<off_bits-1; i++ ){
      if( (i^type)&1 )
        fprintf(ofp,   "        getbitm\n");
      else
        fprintf(ofp,   "        add     a, a\n");
      fprintf(ofp,     "        rl      d\n");
    }
    if( (i^type)&1 )
      fprintf(ofp,     "        getbitm\n");
    else
      fprintf(ofp,     "        add     a, a\n");
    fprintf(ofp,       "        ccf\n"
                       "        jr      c, offnd%c\n"
                       "        inc     d\n"
                       "offnd%c: rr      e\n"
                       "        ex      (sp), hl\n",
                        (off_bits^type)&1?'o':'e', (off_bits^type)&1?'o':'e');
    if( back )
      fprintf(ofp,     "        ex      de, hl\n"
                       "        adc     hl, de\n"
                       "        lddr\n");
    else
      fprintf(ofp,     "        push    hl\n"
                       "        sbc     hl, de\n"
                       "        pop     de\n"
                       "        ldir\n");
    fprintf(ofp,       "        pop     hl\n"
                       "        add     a, a\n");
    if( speed>3 ){
      if( (off_bits^type)&1 )
        fprintf(ofp,   "        jr      z, mailao\n"
                       "        jr      nc, copbyo\n"
                       "        ld      c, %d\n"
                       "        jp      maisio\n", 2-(type&1));
      else
        fprintf(ofp,   "        jr      nc, copbye\n"
                       "        ld      c, %d\n"
                       "        jp      maisie\n", 2-(type&1));
    }
    else{
      if( (off_bits^type)&1 )
        fprintf(ofp,   "        jr      z, mailao\n"
                       "        jr      c, maicoo\n"
                       "        j%c      copbyo\n", speed==2 ? 'r' : 'p');
      else
        fprintf(ofp,   "        jr      c, maicoe\n"
                       "        j%c      copbye\n", speed==2 ? 'r' : 'p');
    }
    fprintf(ofp,       "exitdz: pop     hl\n");
    if( speed==2 )
      fprintf(ofp,     "getbit: ld      a, (hl)\n"
                       "        %sc     hl\n"
                       "        adc     a, a\n", back?"de":"in");
    fprintf(ofp,       "        ret\n"
                       "mailao: ld      a, (hl)\n"
                       "        %sc     hl\n"
                       "        adc     a, a\n"
                       "        jr      nc, copbyo\n", back?"de":"in");
    if( type&1 )
      fprintf(ofp,     "maicoo: ld      bc, 1\n"
                       "        push    de\n"
                       "        ld      d, b\n"
                       "levalo: add     a, a\n"
                       "        rl      c\n"
                       "        jr      z, exitdz\n"
                       "        rl      b\n"
                       "        getbitm\n"
                       "        jr      nc, levalo\n"
                       "        ld      e, (hl)\n");
    else
      fprintf(ofp,     "maicoo: ld      bc, 2\n"
                       "        push    de\n"
                       "        ld      d, b\n"
                       "        add     a, a\n"
                       "        j%c      c, contio\n"
                       "        dec     c\n"
                       "levalo: getbitm\n"
                       "        rl      c\n"
                       "        rl      b\n"
                       "        add     a, a\n"
                       "        jr      nc, levalo\n"
                       "        inc     c\n"
                       "        jr      z, exitdz\n"
                       "contio: ld      e, (hl)\n", speed==2 ? 'r' : 'p');
    fprintf(ofp,       "        %sc     hl\n"
                       "        defb    203, 51\n" // sll e
                       "        jr      nc, offnd%c\n", back?"de":"in", type&1?'e':'o');
    for ( i=8; i<off_bits-1; i++ ){
      if( (i^type)&1 )
        fprintf(ofp,   "        add     a, a\n");
      else
        fprintf(ofp,   "        getbitm\n");
      fprintf(ofp,     "        rl      d\n");
    }
    if( (i^type)&1 )
      fprintf(ofp,     "        add     a, a\n");
    else
      fprintf(ofp,     "        getbitm\n");
    fprintf(ofp,       "        ccf\n"
                       "        jr      c, offnd%c\n"
                       "        inc     d\n"
                       "offnd%c: rr      e\n"
                       "        ex      (sp), hl\n",
                        (off_bits^type)&1?'e':'o', (off_bits^type)&1?'e':'o');
    if( back )
      fprintf(ofp,     "        ex      de, hl\n"
                       "        adc     hl, de\n"
                       "        lddr\n");
    else
      fprintf(ofp,     "        push    hl\n"
                       "        sbc     hl, de\n"
                       "        pop     de\n"
                       "        ldir\n");
    fprintf(ofp,       "        pop     hl\n"
                       "        add     a,  a\n");
    if( (off_bits^type)&1 )
      fprintf(ofp,     "        jp      c, maicoe\n"
                       "        jp      copbye\n");
    else
      fprintf(ofp,     "        jr      z, mailao\n"
                       "        jr      c, maicoo\n"
                       "        jp      copbyo\n");
  }
  return 0;
}
