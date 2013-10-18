!UBX CFG-RATE 1000 1 1      # set update period in milliseconds
!UBX CFG-MSG 2 17 0 0 0 1   # RXM RAW - required for rtklib
!UBX CFG-MSG 2 16 0 0 0 1   # RXM SFRB - recommended for rtklib
!UBX CFG-MSG 240 0 0 0 0 0  # NMEA GGA
!UBX CFG-MSG 240 1 0 0 0 0  # NMEA GLL
!UBX CFG-MSG 240 2 0 0 0 0  # NMEA GSA
!UBX CFG-MSG 240 3 0 0 0 0  # NMEA GSV 
!UBX CFG-MSG 240 4 0 0 0 0  # NMEA RMC
!UBX CFG-MSG 240 5 0 0 0 0  # NMEA VTG
!UBX CFG-MSG 240 8 0 0 0 0  # NMEA ZDA

