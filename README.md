# Linker
Cilj ovog zadatka jeste realizacija linkera nezavisnog od ciljne arhitekture koji vrši povezivanje predmetnih programa, generisanih od strane [asemblera](https://github.com/n-itrogenium/Two_Pass_Assembler), na osnovu
metapodataka (tabela simbola, relokacioni zapisi, sekcije). Ulaz linkera je izlaz asemblera, pri čemu je moguće zadati veći broj predmetnih programa koje je potrebno povezati. Izlaz linkera je
tekstualna datoteka sa sadržajem u skladu sa opisom u nastavku.


## Prevođenje i pokretanje projekta iz terminala
Prevođenje projekta se vrši zadavanjem sledeće komande:
```
g++ -o linker src/linker.cpp src/main.cpp src/section.cpp src/symboltable.cpp -I inc
```
Jednim pokretanjem `linker` vrši povezivanje jedne ili više ulaznih datoteka. Nazivi ulaznih datoteka, koje predstavljaju predmetne programe, zadaju se kao samostalni argumenti komandne linije.
Način pokretanja linker jeste sledeći:
```
./linker [opcije] -o <naziv_izlazne_datoteke> <niz_naziva_ulaznih_datoteka>
```
Opcija komandne linije `-o` postavlja svoj parametar `<naziv_izlazne_datoteke>` za naziv izlazne datoteke koja predstavlja rezultat linkovanja.

##### Dodatne opcije komandne linije
```
–place=<ime_sekcije>@<adresa>
```
Opcija komandne linije `-place` definiše adresu počev od koje se smešta sekcija zadatog imena, pri čemu su adresa i ime sekcije određeni njenim `<ime_sekcije>@<adresa>` parametrom. Ovu opciju moguće je navoditi veći broj puta, kako bi se definisala adresa za svaku od sekcija iz ulaznih datoteka. Sekcije za koje ova opcija nije navedena smeštaju se proizvoljnim redom odmah iza sekcije koja je smeštena na najvišu adresu. Ukoliko u više ulaznih datoteka postoje sekcije istog imena, njihov međusobni položaj u okviru istoimene agregirane sekcije je proizvoljan.

```
-hex
```
Opcija komandne linije `-hex` predstavlja smernicu linkeru da kao rezultat povezivanja generiše zapis na osnovu kojeg se može izvršiti inicijalizacija memorije u vidu skupa parova _adresa:sadržaj_ (primer: `0000: 00 01 02 03 04 05 06 07`). Povezivanje je moguće samo u slučaju da ne postoje višestruke definicije simbola, nerazrešeni simboli i preklapanja između sekcija iz ulaznih predmetnih programa kada se uzmu u obzir `-place` opcije komandne linije.

```
-linkable
```
Opcija komandne linije `-linkable` predstavlja smernicu linkeru da kao rezultat povezivanja generiše predmetni program istog formata kao i izlaz asemblera, u kojem se sve
sekcije smeštaju takođe od nulte adrese (potpuno se ignorišu potencijalno navedene `-place` opcije komandne linije). Predmetni program dobijen na ovakav način može kasnije biti naveden kao ulaz linkera. Povezivanje je moguće samo u slučaju da nema višestrukih definicija simbola. 

##### Primer pokretanja
Prilikom pokretanja linkera, navođenje tačno jedne od `-linkable` i `-hex` opcija komandne linije je obavezno. Sledeće komande su iskorišćene za generisanje izlaznih datoteka u nastavku.
```
./linker -hex -place=ivt@0x1000 -o mem_content.hex interrupts.o main.o
./linker -linkable -o linked.o interrupts.o main.o
```


## Rezultati linkovanja
U nastavku su date ulazne test datoteke zajedno sa svojim rezultujućim datotekama nakon linkovanja, koje su u obliku sadržaja memorije ili predmetnog programa, u zavisnosti od toga koje su opcije zadate.

##### interrupts.o
```
============================SYMBOL TABLE==============================
          Label        Section     Value      Size     Scope   Ordinal
----------------------------------------------------------------------
            isr            isr         0        62         L         1
            ivt            ivt         0        16         L         2
      asciiCode            abs        54                   L         3
      isr_reset            isr         0                   L         4
   isr_terminal            isr        16                   L         5
      isr_timer            isr         5                   L         6
      myCounter              ?         0                   G         7
        myStart              ?         0                   G         8
        term_in            abs      ff02                   L         9
       term_out            abs      ff00                   L        10



===========================RELOCATION TABLE===========================
Section: isr
    Offset   Rel. type   Ordinal
--------------------------------
         3         ABS         8
        41      PC_REL         7
        53         ABS         7



Section: ivt
    Offset   Rel. type   Ordinal
--------------------------------
         0         ABS         1
         4         ABS         1
         6         ABS         1



===============================SECTIONS===============================
Section: isr	[62 bytes]
-----------------------------------------------
50 f0 00 00 00 b0 60 12 a0 00 00 00 54 b0 00 04
ff 00 a0 06 42 20 b0 60 12 b0 61 12 a0 00 04 ff
02 b0 00 04 ff 00 a0 07 03 ff fe a0 10 00 00 01
70 01 b0 00 04 00 00 a0 16 42 a0 06 42 20 

Section: ivt	[16 bytes]
-----------------------------------------------
00 00 00 00 05 00 16 00 00 00 00 00 00 00 00 00
```

##### main.o
```
============================SYMBOL TABLE==============================
          Label        Section     Value      Size     Scope   Ordinal
----------------------------------------------------------------------
         myCode         myCode         0        28         L         1
         myData         myData         0         2         L         2
      myCounter         myData         0                   G         3
        myStart         myCode         0                   G         4
        tim_cfg            abs      ff10                   L         5
           wait         myCode         a                   L         6



===========================RELOCATION TABLE===========================
Section: myCode
    Offset   Rel. type   Ordinal
--------------------------------
        13         ABS         2
        25         ABS         1



===============================SECTIONS===============================
Section: myCode	[28 bytes]
-----------------------------------------------
a0 00 00 00 01 b0 00 04 ff 10 a0 10 04 00 00 a0
10 00 00 05 74 01 52 f0 00 00 0a 00 

Section: myData	[2 bytes]
-----------------------------------------------
00 00 
```

##### mem_content.hex
```
1000: 10 10 00 00 15 10 26 10
1008: 00 00 00 00 00 00 00 00
1010: 50 f0 00 10 4e b0 60 12
1018: a0 00 00 00 54 b0 00 04
1020: ff 00 a0 06 42 20 b0 60
1028: 12 b0 61 12 a0 00 04 ff
1030: 02 b0 00 04 ff 00 a0 07
1038: 03 00 2f a0 10 00 00 01
1040: 70 01 b0 00 04 10 6a a0
1048: 16 42 a0 06 42 20 a0 00
1050: 00 00 01 b0 00 04 ff 10
1058: a0 10 04 10 6a a0 10 00
1060: 00 05 74 01 52 f0 00 10
1068: 58 00 00 00 
```

##### linked.o
```
============================SYMBOL TABLE==============================
          Label        Section     Value      Size     Scope   Ordinal
----------------------------------------------------------------------
            isr            isr         0        62         L         1
            ivt            ivt         0        16         L         2
         myCode         myCode         0        28         L         3
         myData         myData         0         2         L         4
      myCounter         myData         0                   G         5
        myStart         myCode         0                   G         6



===========================RELOCATION TABLE===========================
Section: isr
    Offset   Rel. type   Ordinal
--------------------------------
         3         ABS         3
        41      PC_REL         4
        53         ABS         4



Section: ivt
    Offset   Rel. type   Ordinal
--------------------------------
         0         ABS         1
         4         ABS         1
         6         ABS         1



Section: myCode
    Offset   Rel. type   Ordinal
--------------------------------
        13         ABS         4
        25         ABS         3



===============================SECTIONS===============================
Section: isr	[62 bytes]
-----------------------------------------------
50 f0 00 00 00 b0 60 12 a0 00 00 00 54 b0 00 04
ff 00 a0 06 42 20 b0 60 12 b0 61 12 a0 00 04 ff
02 b0 00 04 ff 00 a0 07 03 ff fe a0 10 00 00 01
70 01 b0 00 04 00 00 a0 16 42 a0 06 42 20 

Section: ivt	[16 bytes]
-----------------------------------------------
00 00 00 00 05 00 16 00 00 00 00 00 00 00 00 00


Section: myCode	[28 bytes]
-----------------------------------------------
a0 00 00 00 01 b0 00 04 ff 10 a0 10 04 00 00 a0
10 00 00 05 74 01 52 f0 00 00 0a 00 

Section: myData	[2 bytes]
-----------------------------------------------
00 00 
```


